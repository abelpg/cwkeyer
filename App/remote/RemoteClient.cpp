#include "RemoteClient.h"

#include "../utils/Logger.h"
#include "../utils/Utils.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

/// Encodes a byte buffer as Base64 (used for the Sec-WebSocket-Key header).
std::string base64Encode(const uint8_t *data, size_t len) {
  static constexpr char table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::string out;
  out.reserve(((len + 2) / 3) * 4);

  for (size_t i = 0; i < len; i += 3) {
    const uint32_t octetA = data[i];
    const uint32_t octetB = (i + 1 < len) ? data[i + 1] : 0;
    const uint32_t octetC = (i + 2 < len) ? data[i + 2] : 0;
    const uint32_t triple = (octetA << 16) | (octetB << 8) | octetC;

    out.push_back(table[(triple >> 18) & 0x3F]);
    out.push_back(table[(triple >> 12) & 0x3F]);
    out.push_back((i + 1 < len) ? table[(triple >> 6) & 0x3F] : '=');
    out.push_back((i + 2 < len) ? table[triple & 0x3F] : '=');
  }

  return out;
}

/// Returns true if the HTTP response headers correspond to a successful WebSocket upgrade.
bool isWebSocketUpgradeResponse(const std::string &headers) {
  if (headers.find("HTTP/1.1 101") != 0) {
    return false;
  }
  return headers.find("Upgrade: websocket") != std::string::npos ||
         headers.find("upgrade: websocket") != std::string::npos;
}

} // namespace

RemoteClient::RemoteClient() = default;

/// Ensures the connection and threads are torn down on destruction.
RemoteClient::~RemoteClient() {
  stop();
}

/// Validates parameters, opens the TCP connection, performs the WebSocket
/// handshake and starts the MOX timer thread.
bool RemoteClient::start(const std::string &serverIp, int port, int moxReleaseDelayMs) {
  if (port <= 0 || port > 65535) {
    log(L_ERROR) << "RemoteClient::start() invalid port: " << port;
    return false;
  }
  if (serverIp.empty()) {
    log(L_ERROR) << "RemoteClient::start() invalid IP: " << serverIp;
    return false;
  }

  stop();

  if (!platformInit()) {
    return false;
  }

  if (!openConnection(serverIp, port)) {
    return false;
  }

  if (!performWebSocketHandshake(serverIp, port)) {
    closeConnection();
    return false;
  }

  {
    std::lock_guard lock(m_moxMutex);
    resetMoxStateLocked(moxReleaseDelayMs);
  }

  m_running = true;
  m_moxTimerThread = std::thread(&RemoteClient::moxTimerLoop, this);
  log(L_INFO) << "RemoteClient connected to " << serverIp << ":" << port;
  return true;
}

/// Stops the timer thread, releases MOX if it was active, closes the socket
/// and performs platform cleanup.
void RemoteClient::stop() {
  bool shouldDeactivateMox = false;
  {
    std::lock_guard lock(m_moxMutex);
    shouldDeactivateMox = m_moxActive;
    m_stopMoxTimerThread = true;
    m_moxDeactivationScheduled = false;
    ++m_moxScheduleToken;
    m_moxDeactivationAtMs = 0;
  }
  m_moxCv.notify_all();

  if (m_moxTimerThread.joinable()) {
    m_moxTimerThread.join();
  }

  if (m_running && shouldDeactivateMox && m_socketFd != -1) {
    std::lock_guard lock(m_moxMutex);
    deactivateMoxLocked();
  }

  {
    std::lock_guard lock(m_moxMutex);
    m_moxActive = false;
    m_stopMoxTimerThread = false;
  }

  m_running = false;

  closeConnection();
  platformCleanup();
}

/// Returns true while the client is connected and operational.
bool RemoteClient::started() const {
  return m_running.load();
}

/// Sends a timed CW element (dit/dah); other item types are ignored.
void RemoteClient::runCW(KeyerItem item, int duration) {
  if (item != DIT && item != DAH) {
    return;
  }
  sendDuration(duration);
}

/// Presses the remote key (key down).
void RemoteClient::startRunCw() {
  sendCommand(true);
}

/// Releases the remote key (key up).
void RemoteClient::stopRunCw() {
  sendCommand(false);
}

/// Validates and forwards a keyer element duration to the server.
bool RemoteClient::sendDuration(int duration) {
  if (!m_running || duration <= 0) {
    return false;
  }

  return sendTimedCommand(duration);
}

/// Builds and sends a timed keyer command ("keyer:0,true,<duration>;").
bool RemoteClient::sendTimedCommand(int duration) {
  return sendKeyerCommand("keyer:0,true," + std::to_string(duration) + ";", duration);
}

/// Builds and sends an untimed keyer command (key down / key up).
bool RemoteClient::sendCommand(bool keyDown) {
  const std::string command = keyDown ? "keyer:0,true;" : "keyer:0,false;";
  return sendKeyerCommand(command, 0);
}

/// Sends a keyer command, activating MOX beforehand if needed, and either
/// releases MOX immediately (no delay configured) or schedules its release.
bool RemoteClient::sendKeyerCommand(const std::string &command, int duration) {
  std::lock_guard lock(m_moxMutex);

  if (!m_running || m_socketFd == -1) {
    return false;
  }

  // Cancel any pending deactivation: a new command extends MOX.
  m_moxDeactivationScheduled = false;
  ++m_moxScheduleToken;

  if (!m_moxActive && !activateMoxLocked()) {
    return false;
  }

  if (!sendLine(command)) {
    return false;
  }

  if (m_moxReleaseDelayMs <= 0) {
    deactivateMoxLocked();
    return m_running;
  }

  scheduleMoxReleaseLocked(duration);
  return true;
}

/// Activates MOX by keying the TRX.
/// Must be called with m_moxMutex held.
bool RemoteClient::activateMoxLocked() {
  if (!sendLine("trx:0,true;")) {
    return false;
  }

  m_moxActive = true;
  return true;
}

/// Deactivates MOX by unkeying the TRX.
/// Must be called with m_moxMutex held.
void RemoteClient::deactivateMoxLocked() {
  sendLine("trx:0,false;");

  m_moxActive = false;
}

/// Schedules the MOX release after the configured delay plus the element duration.
/// Must be called with m_moxMutex held.
void RemoteClient::scheduleMoxReleaseLocked(int duration) {
  m_moxDeactivationScheduled = true;
  m_moxDeactivationAtMs = nowMs() + static_cast<uint64_t>(m_moxReleaseDelayMs + duration);
  ++m_moxScheduleToken;
  m_moxCv.notify_one();
}

/// Resets all MOX-related state to a clean initial condition.
/// Must be called with m_moxMutex held.
void RemoteClient::resetMoxStateLocked(int moxReleaseDelayMs) {
  m_moxReleaseDelayMs = moxReleaseDelayMs >= 0 ? moxReleaseDelayMs : 0;
  m_moxActive = false;
  m_stopMoxTimerThread = false;
  m_moxDeactivationScheduled = false;
  ++m_moxScheduleToken;
  m_moxDeactivationAtMs = 0;
}

/// Background loop that releases MOX when the scheduled deadline expires,
/// unless a newer command reschedules or cancels the deactivation.
void RemoteClient::moxTimerLoop() {
  std::unique_lock lock(m_moxMutex);

  while (!m_stopMoxTimerThread) {
    m_moxCv.wait(lock, [this]() { return m_stopMoxTimerThread || m_moxDeactivationScheduled; });
    if (m_stopMoxTimerThread) {
      break;
    }

    const uint64_t token = m_moxScheduleToken;
    const uint64_t deactivateAtMs = m_moxDeactivationAtMs;
    const uint64_t currentMs = nowMs();

    if (deactivateAtMs > currentMs) {
      const auto waitDuration = std::chrono::milliseconds(deactivateAtMs - currentMs);
      const bool interrupted = m_moxCv.wait_for(lock, waitDuration, [this, token]() {
        return m_stopMoxTimerThread || !m_moxDeactivationScheduled || m_moxScheduleToken != token;
      });
      if (interrupted) {
        continue; // Rescheduled, cancelled or stopping: re-evaluate from the top.
      }
    }

    m_moxDeactivationScheduled = false;
    if (m_moxActive) {
      deactivateMoxLocked();
    }
  }
}

/// Encodes the text as a masked WebSocket text frame and sends it to the server.
bool RemoteClient::sendLine(const std::string &line) {
  std::lock_guard lock(m_sendMutex);

  if (!m_running || m_socketFd == -1) {
    return false;
  }

  std::vector<uint8_t> frame;
  frame.reserve(line.size() + 14);
  frame.push_back(0x81); // FIN + text frame

  // Payload length (7 bits, 16 bits or 64 bits) with the mask bit set.
  if (line.size() <= 125) {
    frame.push_back(static_cast<uint8_t>(0x80 | line.size()));
  } else if (line.size() <= 0xFFFF) {
    frame.push_back(0x80 | 126);
    frame.push_back(static_cast<uint8_t>((line.size() >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(line.size() & 0xFF));
  } else {
    frame.push_back(0x80 | 127);
    for (int i = 7; i >= 0; --i) {
      frame.push_back(static_cast<uint8_t>((line.size() >> (8 * i)) & 0xFF));
    }
  }

  // Client frames must be masked (RFC 6455).
  std::array<uint8_t, 4> maskKey{};
  for (auto &byte : maskKey) {
    byte = static_cast<uint8_t>(std::rand() & 0xFF);
    frame.push_back(byte);
  }

  for (size_t i = 0; i < line.size(); ++i) {
    frame.push_back(static_cast<uint8_t>(line[i]) ^ maskKey[i % 4]);
  }

  if (!sendRaw(frame.data(), frame.size())) {
    m_running = false;
    return false;
  }

  return true;
}

/// Performs the HTTP upgrade handshake required to open the WebSocket.
bool RemoteClient::performWebSocketHandshake(const std::string &serverIp, int port) {
  std::array<uint8_t, 16> randomBytes{};
  for (auto &byte : randomBytes) {
    byte = static_cast<uint8_t>(std::rand() & 0xFF);
  }

  const std::string wsKey = base64Encode(randomBytes.data(), randomBytes.size());
  const std::string request =
      "GET / HTTP/1.1\r\n"
      "Host: " + serverIp + ":" + std::to_string(port) + "\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Key: " + wsKey + "\r\n"
      "Sec-WebSocket-Version: 13\r\n\r\n";

  if (!sendRaw(request.data(), request.size())) {
    return false;
  }

  std::string response;
  response.reserve(1024);
  std::array<char, 512> buffer{};
  while (response.find("\r\n\r\n") == std::string::npos) {
    const int received = recvRaw(buffer.data(), buffer.size(), -1);
    if (received <= 0) {
      return false;
    }
    response.append(buffer.data(), static_cast<size_t>(received));
    if (response.size() > 8192) {
      return false;
    }
  }

  return isWebSocketUpgradeResponse(response);
}


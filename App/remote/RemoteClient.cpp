#include "RemoteClient.h"

#include "../utils/Logger.h"
#include "../utils/Utils.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

constexpr int WS_READ_TIMEOUT_MS = 1000;
constexpr uint64_t WS_PING_INTERVAL_MS = 15000;
constexpr uint64_t WS_MAX_PAYLOAD_LEN = 1024 * 1024;
/// Constant settle time to wait after enabling MOX before keying.

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

  m_running = true;
  m_stopWebSocketThread = false;
  m_stopSenderThread = false;
  m_webSocketThread = std::thread(&RemoteClient::webSocketLoop, this);
  m_senderThread = std::thread(&RemoteClient::senderLoop, this);
  log(L_INFO) << "RemoteClient connected to " << serverIp << ":" << port;
  return true;
}

/// Stops the timer thread, releases MOX if it was active, closes the socket
/// and performs platform cleanup.
void RemoteClient::stop() {

  m_stopWebSocketThread = true;
  m_stopSenderThread = true;
  m_queueCv.notify_all();

  if (m_senderThread.joinable()) {
    m_senderThread.join();
  }

  if (m_webSocketThread.joinable()) {
    m_webSocketThread.join();
  }


  {
    std::lock_guard lock(m_queueMutex);
    std::queue<CwElement> empty;
    m_cwQueue.swap(empty);
  }

  m_running = false;

  closeConnection();
  platformCleanup();
}

/// Returns true while the client is connected and operational.
bool RemoteClient::started() const {
  return m_running.load();
}

int RemoteClient::calculateSpaceBetweenCwElement(int spaceDuration) {
  const uint64_t now = nowMs();
  if (m_lastSendKeyerCommandAtMs != 0 && now >= m_lastSendKeyerCommandAtMs) {
    const uint64_t elapsed = now - m_lastSendKeyerCommandAtMs;
    if (elapsed <= (spaceDuration * 3) + 10) {
      return static_cast<int>(elapsed);
    }
  }
  return 0;
}

/// Enqueues a timed CW element (dit/dah); other item types are ignored.
void RemoteClient::runCW(KeyerItem item, int duration, int spaceDuration) {
  if (item != DIT && item != DAH) {
    return;
  }
  if (!m_running || duration <= 0) {
    return;
  }

  // Space between calls.
  int spaceBetweenCwElement = calculateSpaceBetweenCwElement(spaceDuration);

  // Send dit/dah
  {
    std::lock_guard lock(m_queueMutex);
    m_cwQueue.push(CwElement{duration, spaceDuration,spaceBetweenCwElement});
    m_lastSendKeyerCommandAtMs = nowMs();
  }
  m_queueCv.notify_one();
}

/// Presses the remote key (key down).
void RemoteClient::startRunCw() {
  sendCommand(true);
}

/// Releases the remote key (key up).
void RemoteClient::stopRunCw() {
  sendCommand(false);
}

/// Worker loop: waits for queued CW elements and sends them one by one.
void RemoteClient::senderLoop() {
  while (!m_stopSenderThread.load()) {
    CwElement element{};
    {
      std::unique_lock lock(m_queueMutex);
      m_queueCv.wait(lock, [this]() {
        return m_stopSenderThread.load() || !m_cwQueue.empty();
      });
      if (m_stopSenderThread.load()) {
        break;
      }
      if (m_cwQueue.empty()) {
        continue;
      }
      element = m_cwQueue.front();
      m_cwQueue.pop();
    }

    if (element.spaceBetweenCwElement > 0) {
      Utils::sleepFor(element.spaceBetweenCwElement);
    }

    const bool resultDown = sendKeyerCommand(true, element.duration - 20);
    if (resultDown) {
      Utils::sleepFor(element.duration);
    }
    const bool resultUp = resultDown && sendKeyerCommand(false, 0);

    if (resultUp) {
      Utils::sleepFor(element.spaceDuration );
    }

  }
}



/// Builds and sends an untimed keyer command (key down / key up).
bool RemoteClient::sendCommand(bool keyDown) {
  return sendKeyerCommand(keyDown, 0);
}

/// Sends a keyer command over the WebSocket.
bool RemoteClient::sendKeyerCommand(bool keyDown, int intervalMs) {
  if (intervalMs < 0) {
    intervalMs = 0;
  }

  if (!m_running || m_socketFd == -1) {
    return false;
  }

  const std::string command =
      "keyer:0," + std::string(keyDown ? "true" : "false") + "," + std::to_string(intervalMs) + ";";

  return sendLine(command);
}

/// Sends a masked WebSocket frame with the given opcode and payload.
bool RemoteClient::sendFrame(uint8_t opcode, const uint8_t *payload, size_t payloadLen) {
  std::lock_guard lock(m_sendMutex);

  if (!m_running || m_socketFd == -1) {
    return false;
  }

  std::vector<uint8_t> frame;
  frame.reserve(payloadLen + 14);
  frame.push_back(static_cast<uint8_t>(0x80 | (opcode & 0x0F))); // FIN + opcode

  // Payload length (7 bits, 16 bits or 64 bits) with the mask bit set.
  if (payloadLen <= 125) {
    frame.push_back(static_cast<uint8_t>(0x80 | payloadLen));
  } else if (payloadLen <= 0xFFFF) {
    frame.push_back(0x80 | 126);
    frame.push_back(static_cast<uint8_t>((payloadLen >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
  } else {
    frame.push_back(0x80 | 127);
    for (int i = 7; i >= 0; --i) {
      frame.push_back(static_cast<uint8_t>((payloadLen >> (8 * i)) & 0xFF));
    }
  }

  // Client frames must be masked (RFC 6455).
  std::array<uint8_t, 4> maskKey{};
  for (auto &byte : maskKey) {
    byte = static_cast<uint8_t>(std::rand() & 0xFF);
    frame.push_back(byte);
  }

  for (size_t i = 0; i < payloadLen; ++i) {
    frame.push_back(payload[i] ^ maskKey[i % 4]);
  }

  if (!sendRaw(frame.data(), frame.size())) {
    m_running = false;
    return false;
  }

  return true;
}

/// Sends a WebSocket control frame (ping/pong/close).
bool RemoteClient::sendControlFrame(uint8_t opcode, const std::vector<uint8_t> &payload) {
  return sendFrame(opcode, payload.data(), payload.size());
}


/// Encodes the text as a masked WebSocket text frame and sends it to the server.
bool RemoteClient::sendLine(const std::string &line) {

  auto now= nowMs();

  bool result = sendFrame(0x1, reinterpret_cast<const uint8_t *>(line.data()), line.size());
  log(L_INFO) << "Sending line: " << line << " in " << (nowMs() - now) << " ms";
  return result;
}

/// Reads exactly len bytes with timeout handling.
/// Returns 1 on success, -2 on timeout, 0 on close and -1 on error.
int RemoteClient::readExact(uint8_t *buffer, size_t len, int timeoutMs) {
  size_t offset = 0;
  while (offset < len && m_running && !m_stopWebSocketThread.load()) {
    const int received = recvRaw(buffer + offset, len - offset, timeoutMs);
    if (received > 0) {
      offset += static_cast<size_t>(received);
      continue;
    }
    if (received == -2) {
      if (offset == 0) {
        return -2;
      }
      continue;
    }
    return received;
  }

  return offset == len ? 1 : -1;
}

/// Reads a single WebSocket frame.
/// Returns 1 on success, -2 on timeout, 0 on close and -1 on error/protocol error.
int RemoteClient::recvFrame(uint8_t &opcode, std::vector<uint8_t> &payload, int timeoutMs) {
  std::array<uint8_t, 2> header{};
  const int headerStatus = readExact(header.data(), header.size(), timeoutMs);
  if (headerStatus != 1) {
    return headerStatus;
  }

  const bool isFinal = (header[0] & 0x80) != 0;
  opcode = static_cast<uint8_t>(header[0] & 0x0F);
  const bool hasMask = (header[1] & 0x80) != 0;
  uint64_t payloadLen = static_cast<uint64_t>(header[1] & 0x7F);

  if (!isFinal) {
    return -1;
  }

  if (payloadLen == 126) {
    std::array<uint8_t, 2> extLen{};
    const int extStatus = readExact(extLen.data(), extLen.size(), timeoutMs);
    if (extStatus != 1) {
      return extStatus;
    }
    payloadLen = (static_cast<uint64_t>(extLen[0]) << 8) | static_cast<uint64_t>(extLen[1]);
  } else if (payloadLen == 127) {
    std::array<uint8_t, 8> extLen{};
    const int extStatus = readExact(extLen.data(), extLen.size(), timeoutMs);
    if (extStatus != 1) {
      return extStatus;
    }
    payloadLen = 0;
    for (uint8_t byte : extLen) {
      payloadLen = (payloadLen << 8) | static_cast<uint64_t>(byte);
    }
  }

  if (payloadLen > WS_MAX_PAYLOAD_LEN) {
    return -1;
  }

  std::array<uint8_t, 4> maskKey{};
  if (hasMask) {
    const int maskStatus = readExact(maskKey.data(), maskKey.size(), timeoutMs);
    if (maskStatus != 1) {
      return maskStatus;
    }
  }

  payload.assign(static_cast<size_t>(payloadLen), 0);
  if (payloadLen > 0) {
    const int payloadStatus = readExact(payload.data(), payload.size(), timeoutMs);
    if (payloadStatus != 1) {
      return payloadStatus;
    }
    if (hasMask) {
      for (size_t i = 0; i < payload.size(); ++i) {
        payload[i] ^= maskKey[i % 4];
      }
    }
  }

  return 1;
}

/// Receives server control frames and handles ping/pong keepalive.
void RemoteClient::webSocketLoop() {
  uint64_t lastPingMs = nowMs();

  while (m_running && !m_stopWebSocketThread.load()) {
    uint8_t opcode = 0;
    std::vector<uint8_t> payload;
    const int frameStatus = recvFrame(opcode, payload, WS_READ_TIMEOUT_MS);

    if (frameStatus == -2) {
      if (nowMs() - lastPingMs >= WS_PING_INTERVAL_MS) {
        if (!sendControlFrame(0x9, {})) {
          m_running = false;
          break;
        }
        lastPingMs = nowMs();
      }
      continue;
    }

    if (frameStatus <= 0) {
      m_running = false;
      break;
    }

    switch (opcode) {
      case 0x8: // close
        sendControlFrame(0x8, payload);
        m_running = false;
        return;
      case 0x9: // ping
        if (!sendControlFrame(0xA, payload)) {
          m_running = false;
          return;
        }
        break;
      case 0xA: // pong
      default:
        break;
    }
  }
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


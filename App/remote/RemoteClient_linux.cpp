#include "RemoteClient.h"

#include "../utils/Logger.h"
#include "../utils/Utils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

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

bool isWebSocketUpgradeResponse(const std::string &headers) {
  if (headers.find("HTTP/1.1 101") != 0) {
    return false;
  }
  return headers.find("Upgrade: websocket") != std::string::npos ||
         headers.find("upgrade: websocket") != std::string::npos;
}

} // namespace

RemoteClient::RemoteClient() = default;

RemoteClient::~RemoteClient() {
  stop();
}

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

  const auto sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return false;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(static_cast<uint16_t>(port));

  if (::inet_pton(AF_INET, serverIp.c_str(), &address.sin_addr) <= 0) {
    ::close(sock);
    return false;
  }

  if (::connect(sock, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
    ::close(sock);
    return false;
  }

  m_socketFd = static_cast<intptr_t>(sock);
  if (!performWebSocketHandshake(serverIp, port)) {
    ::close(sock);
    m_socketFd = -1;
    return false;
  }

  {
    std::lock_guard lock(m_moxMutex);
    m_moxReleaseDelayMs = moxReleaseDelayMs >= 0 ? moxReleaseDelayMs : 0;
    m_moxActive = false;
    m_stopMoxTimerThread = false;
    m_moxDeactivationScheduled = false;
    ++m_moxScheduleToken;
    m_moxDeactivationAtMs = 0;
  }

  m_running = true;
  m_moxTimerThread = std::thread(&RemoteClient::moxTimerLoop, this);
  log(L_INFO) << "RemoteClient connected to " << serverIp << ":" << port;
  return true;
}

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
    sendLine("trx:0,false;");
  }

  {
    std::lock_guard lock(m_moxMutex);
    m_moxActive = false;
    m_stopMoxTimerThread = false;
  }

  m_running = false;

  if (m_socketFd != -1) {
    ::close(static_cast<int>(m_socketFd));
  }

  m_socketFd = -1;
}

bool RemoteClient::started() const {
  return m_running.load();
}

void RemoteClient::runCW(KeyerItem item, int duration) {
  if (item != DIT && item != DAH) {
    return;
  }
  sendDuration(duration);
}

void RemoteClient::startRunCw() {
  sendCommand(true);
}

void RemoteClient::stopRunCw() {
  sendCommand(false);
}

bool RemoteClient::sendDuration(int duration) {
  if (!m_running || duration <= 0) {
    return false;
  }

  return sendTimedCommand(duration);
}

bool RemoteClient::sendTimedCommand(int duration) {
  return sendKeyerCommand("keyer:0,true," + std::to_string(duration) + ";");
}

bool RemoteClient::sendCommand(bool keyDown) {
  if (keyDown) {
    return sendKeyerCommand("keyer:0,true;");
  }
  return sendKeyerCommand("keyer:0,false;");
}

bool RemoteClient::sendKeyerCommand(const std::string &command) {
  std::lock_guard lock(m_moxMutex);

  if (!m_running || m_socketFd == -1) {
    return false;
  }

  m_moxDeactivationScheduled = false;
  ++m_moxScheduleToken;

  if (!m_moxActive) {
    if (!sendLine("trx:0,true;")) {
      return false;
    }
    m_moxActive = true;
  }

  if (!sendLine(command)) {
    return false;
  }

  if (m_moxReleaseDelayMs <= 0) {
    if (!sendLine("trx:0,false;")) {
      m_moxActive = false;
      return false;
    }
    m_moxActive = false;
    return true;
  }

  m_moxDeactivationScheduled = true;
  m_moxDeactivationAtMs = nowMs() + static_cast<uint64_t>(m_moxReleaseDelayMs);
  ++m_moxScheduleToken;
  m_moxCv.notify_one();
  return true;
}

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
      if (m_moxCv.wait_for(lock, waitDuration, [this, token]() {
            return m_stopMoxTimerThread || !m_moxDeactivationScheduled || m_moxScheduleToken != token;
          })) {
        continue;
      }
    }

    m_moxDeactivationScheduled = false;
    if (!m_moxActive) {
      continue;
    }

    const bool sent = sendLine("trx:0,false;");

    if (!sent && !m_running) {
      m_moxActive = false;
      continue;
    }

    m_moxActive = false;
  }
}

bool RemoteClient::sendLine(const std::string &line) {
  std::lock_guard lock(m_sendMutex);

  if (!m_running || m_socketFd == -1) {
    return false;
  }

  std::vector<uint8_t> frame;
  frame.reserve(line.size() + 14);
  frame.push_back(0x81); // FIN + text frame

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

  std::array<uint8_t, 4> maskKey{};
  for (auto &byte : maskKey) {
    byte = static_cast<uint8_t>(std::rand() & 0xFF);
    frame.push_back(byte);
  }

  for (size_t i = 0; i < line.size(); ++i) {
    frame.push_back(static_cast<uint8_t>(line[i]) ^ maskKey[i % 4]);
  }

  size_t sentTotal = 0;
  while (sentTotal < frame.size()) {
    const int sent = static_cast<int>(::send(static_cast<int>(m_socketFd),
                                             reinterpret_cast<const char *>(frame.data()) + sentTotal,
                                             frame.size() - sentTotal,
                                             0));
    if (sent <= 0) {
      m_running = false;
      return false;
    }
    sentTotal += static_cast<size_t>(sent);
  }

  return true;
}

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

  size_t sentTotal = 0;
  while (sentTotal < request.size()) {
    const int sent = static_cast<int>(::send(static_cast<int>(m_socketFd),
                                             request.c_str() + sentTotal,
                                             request.size() - sentTotal,
                                             0));
    if (sent <= 0) {
      return false;
    }
    sentTotal += static_cast<size_t>(sent);
  }

  std::string response;
  response.reserve(1024);
  std::array<char, 512> buffer{};
  while (response.find("\r\n\r\n") == std::string::npos) {
    const int received = static_cast<int>(::recv(static_cast<int>(m_socketFd),
                                                 buffer.data(),
                                                 buffer.size(),
                                                 0));
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


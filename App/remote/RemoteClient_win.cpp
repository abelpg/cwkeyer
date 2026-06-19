#include "RemoteClient.h"

#include "../utils/Logger.h"
#include "../utils/Utils.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstring>

#pragma comment(lib, "Ws2_32.lib")

RemoteClient::RemoteClient() = default;

RemoteClient::~RemoteClient() {
  stop();
}

bool RemoteClient::start(const std::string &serverIp, int port) {
  if (port <= 0 || port > 65535) {
    log(L_ERROR) << "RemoteClient::start() invalid port: " << port;
    return false;
  }
  if (serverIp.empty()) {
    log(L_ERROR) << "RemoteClient::start() invalid IP: " << serverIp;
    return false;
  }

  stop();

  if (!initWinsock()) {
    return false;
  }

  const auto sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    return false;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(static_cast<uint16_t>(port));

  if (::inet_pton(AF_INET, serverIp.c_str(), &address.sin_addr) <= 0) {
    closesocket(sock);
    return false;
  }

  if (::connect(sock, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
    closesocket(sock);
    return false;
  }

  m_socketFd = static_cast<intptr_t>(sock);
  m_running = true;
  log(L_INFO) << "RemoteClient connected to " << serverIp << ":" << port;
  return true;
}

void RemoteClient::stop() {
  if (!m_running.exchange(false)) {
    if (m_wsaStarted) {
      WSACleanup();
      m_wsaStarted = false;
    }
    return;
  }

  if (m_socketFd != -1) {
    closesocket(static_cast<SOCKET>(m_socketFd));
  }

  m_socketFd = -1;

  if (m_wsaStarted) {
    WSACleanup();
    m_wsaStarted = false;
  }
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
  m_straightStartMs = nowMs();
}

void RemoteClient::stopRunCw() {
  if (m_straightStartMs == 0) {
    return;
  }

  const uint64_t elapsed = nowMs() - m_straightStartMs;
  m_straightStartMs = 0;
  if (elapsed == 0) {
    return;
  }

  sendDuration(static_cast<int>(elapsed));
}

bool RemoteClient::sendDuration(int duration) {
  if (!m_running || duration <= 0) {
    return false;
  }

  return sendLine(std::to_string(duration) + "\n");
}

bool RemoteClient::sendLine(const std::string &line) {
  std::lock_guard lock(m_sendMutex);

  size_t sentTotal = 0;
  while (sentTotal < line.size()) {
    const int sent = ::send(static_cast<SOCKET>(m_socketFd),
                            line.c_str() + sentTotal,
                            static_cast<int>(line.size() - sentTotal),
                            0);
    if (sent == SOCKET_ERROR || sent == 0) {
      m_running = false;
      return false;
    }
    sentTotal += static_cast<size_t>(sent);
  }

  return true;
}

bool RemoteClient::initWinsock() {
  if (m_wsaStarted) {
    return true;
  }

  WSADATA wsaData{};
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    return false;
  }

  m_wsaStarted = true;
  return true;
}


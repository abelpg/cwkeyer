#include "RemoteClient.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

/// Initializes Winsock (WSAStartup); required once before using sockets on Windows.
bool RemoteClient::platformInit() {
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

/// Releases Winsock (WSACleanup) if it was initialized.
void RemoteClient::platformCleanup() {
  if (m_wsaStarted) {
    WSACleanup();
    m_wsaStarted = false;
  }
}

/// Opens a TCP connection to serverIp:port and stores the descriptor in m_socketFd.
bool RemoteClient::openConnection(const std::string &serverIp, int port) {
  const SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
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
  return true;
}

/// Closes the socket stored in m_socketFd, if open.
void RemoteClient::closeConnection() {
  if (m_socketFd != -1) {
    closesocket(static_cast<SOCKET>(m_socketFd));
    m_socketFd = -1;
  }
}

/// Sends the full buffer over the socket; returns false on any error.
bool RemoteClient::sendRaw(const void *data, size_t len) {
  if (m_socketFd == -1) {
    return false;
  }

  const auto *bytes = static_cast<const char *>(data);
  size_t sentTotal = 0;
  while (sentTotal < len) {
    log(L_DEBUG) << "Sending " << sentTotal << " bytes";
    const int sent = ::send(static_cast<SOCKET>(m_socketFd),
                            bytes + sentTotal,
                            static_cast<int>(len - sentTotal),
                            0);

    log(L_DEBUG) << "Sent " << sent << " bytes";
    if (sent == SOCKET_ERROR || sent == 0) {
      return false;
    }
    sentTotal += static_cast<size_t>(sent);
  }

  return true;
}

/// Receives up to len bytes; waits at most timeoutMs (-1 = blocking).
/// Returns bytes read, -2 on timeout, 0 on connection close and -1 on error.
int RemoteClient::recvRaw(void *buffer, size_t len, int timeoutMs) {
  if (m_socketFd == -1) {
    return -1;
  }

  const SOCKET sock = static_cast<SOCKET>(m_socketFd);

  if (timeoutMs >= 0) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    timeval timeout{};
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

    const int ready = ::select(0, &readSet, nullptr, nullptr, &timeout);
    if (ready == SOCKET_ERROR) {
      return -1;
    }
    if (ready == 0) {
      return -2; // Timeout.
    }
  }

  const int received = ::recv(sock, static_cast<char *>(buffer), static_cast<int>(len), 0);
  if (received == SOCKET_ERROR) {
    return -1;
  }
  return received;
}

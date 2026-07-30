/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#include <cmath>

#include "RemoteClient.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

/// No global network initialization is required on Linux.
bool RemoteClient::platformInit() {
  return true;
}

/// No global network cleanup is required on Linux.
void RemoteClient::platformCleanup() {
}

/// Opens a TCP connection to serverIp:port and stores the descriptor in m_socketFd.
bool RemoteClient::openConnection(const std::string &serverIp, int port) {
  const int sock = ::socket(AF_INET, SOCK_STREAM, 0);
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
  return true;
}

/// Closes the socket stored in m_socketFd, if open.
void RemoteClient::closeConnection() {
  if (m_socketFd != -1) {
    ::close(static_cast<int>(m_socketFd));
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
    const ssize_t sent = ::send(static_cast<int>(m_socketFd),
                                bytes + sentTotal,
                                len - sentTotal,
                                0);
    log(L_DEBUG) << "Sent " << sent << " bytes";
    if (sent <= 0) {
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

  const int sock = static_cast<int>(m_socketFd);

  if (timeoutMs >= 0) {
    pollfd pfd{};
    pfd.fd = sock;
    pfd.events = POLLIN;

    const int ready = ::poll(&pfd, 1, timeoutMs);
    if (ready < 0) {
      return -1;
    }
    if (ready == 0) {
      return -2; // Timeout.
    }
  }

  const ssize_t received = ::recv(sock, buffer, len, 0);
  if (received < 0) {
    return -1;
  }
  return static_cast<int>(received);
}



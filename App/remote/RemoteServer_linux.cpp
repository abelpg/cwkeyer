#include "RemoteServer.h"

#include "../utils/Logger.h"
#include "../utils/Utils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

RemoteServer::RemoteServer(IDitDah *ditDah)
  : m_ditDah(ditDah) {}

RemoteServer::~RemoteServer() {
  stop();
}

bool RemoteServer::start(int port) {
  if (port <= 0 || port > 65535) {
    return false;
  }

  stop();

  m_running = true;
  m_serverThread = std::thread(&RemoteServer::acceptLoop, this, port);
  return true;
}

void RemoteServer::stop() {
  if (!m_running.exchange(false)) {
    return;
  }

  if (m_listenFd != -1) {
    ::close(static_cast<int>(m_listenFd));
  }
  m_listenFd = -1;

  if (m_serverThread.joinable()) {
    m_serverThread.join();
  }
}

bool RemoteServer::started() const {
  return m_running.load();
}

void RemoteServer::acceptLoop(int port) {
  const auto listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
  m_listenFd = static_cast<intptr_t>(listenSocket);
  if (listenSocket < 0) {
    m_running = false;
    return;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(static_cast<uint16_t>(port));

  const int yes = 1;
  setsockopt(static_cast<int>(m_listenFd), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (::bind(static_cast<int>(m_listenFd), reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
    m_running = false;
    return;
  }

  if (::listen(static_cast<int>(m_listenFd), 1) != 0) {
    m_running = false;
    return;
  }

  log(L_INFO) << "RemoteServer listening on port " << port;

  while (m_running) {
    const auto client = ::accept(static_cast<int>(m_listenFd), nullptr, nullptr);
    if (client < 0) {
      if (m_running) {
        log(L_WARNING) << "RemoteServer accept failed";
      }
      continue;
    }

    handleClient(static_cast<intptr_t>(client));
    ::close(client);
  }
}

void RemoteServer::handleClient(intptr_t clientSocket) {
  std::string input;
  char buffer[256] = {0};

  while (m_running) {
    const int readLen = static_cast<int>(::recv(static_cast<int>(clientSocket), buffer, sizeof(buffer), 0));
    if (readLen <= 0) {
      break;
    }

    input.append(buffer, buffer + readLen);

    size_t pos = 0;
    while ((pos = input.find('\n')) != std::string::npos) {
      const std::string line = input.substr(0, pos);
      input.erase(0, pos + 1);

      int duration = 0;
      try {
        duration = std::stoi(line);
      } catch (...) {
        continue;
      }

      if (duration <= 0 || !m_ditDah) {
        continue;
      }

      m_ditDah->onStraight(true);
      Utils::sleepFor(duration);
      m_ditDah->onStraight(false);
    }
  }
}


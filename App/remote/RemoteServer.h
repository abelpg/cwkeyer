#ifndef CWKEYERAPP_REMOTESERVER_H
#define CWKEYERAPP_REMOTESERVER_H

#include "../utils/IDitDah.h"

#include <atomic>
#include <cstdint>
#include <thread>

class RemoteServer {
public:
  explicit RemoteServer(IDitDah *ditDah);
  ~RemoteServer();

  bool start(int port);
  void stop();
  bool started() const;

private:
  void acceptLoop(int port);
  void handleClient(intptr_t clientSocket);

#ifdef _WIN32
  bool initWinsock();
  bool m_wsaStarted = false;
#endif

  IDitDah *m_ditDah = nullptr;
  std::atomic<bool> m_running{false};
  std::thread m_serverThread;
  intptr_t m_listenFd = -1;
};

#endif //CWKEYERAPP_REMOTESERVER_H


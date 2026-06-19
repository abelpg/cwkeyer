#ifndef CWKEYERAPP_REMOTECLIENT_H
#define CWKEYERAPP_REMOTECLIENT_H

#include "../utils/IKeyerCW.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

class RemoteClient : public IKeyerCW {
public:
  RemoteClient();
  ~RemoteClient() override;

  bool start(const std::string &serverIp, int port);
  void stop();
  bool started() const;

  void runCW(KeyerItem item, int duration) override;
  void startRunCw() override;
  void stopRunCw() override;

private:
  bool sendDuration(int duration);
  bool sendLine(const std::string &line);

#ifdef _WIN32
  bool initWinsock();
  bool m_wsaStarted = false;
#endif

  intptr_t m_socketFd = -1;
  std::atomic<bool> m_running{false};
  std::mutex m_sendMutex;
  uint64_t m_straightStartMs = 0;
};

#endif //CWKEYERAPP_REMOTECLIENT_H


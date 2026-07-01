#ifndef CWKEYERAPP_REMOTECLIENT_H
#define CWKEYERAPP_REMOTECLIENT_H

#include "../utils/IKeyerCW.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

class RemoteClient : public IKeyerCW {
public:
  RemoteClient();
  ~RemoteClient() override;

  bool start(const std::string &serverIp, int port, int moxReleaseDelayMs);
  void stop();
  bool started() const;

  void runCW(KeyerItem item, int duration) override;
  void startRunCw() override;
  void stopRunCw() override;

private:
  bool performWebSocketHandshake(const std::string &serverIp, int port);
  void moxTimerLoop();
  bool sendDuration(int duration);
  bool sendTimedCommand(int duration);
  bool sendCommand(bool keyDown);
  bool sendKeyerCommand(const std::string &command, int duration);
  bool sendLine(const std::string &line);

#ifdef _WIN32
  bool initWinsock();
  bool m_wsaStarted = false;
#endif

  intptr_t m_socketFd = -1;
  std::atomic<bool> m_running{false};
  std::mutex m_sendMutex;
  std::mutex m_moxMutex;
  std::condition_variable m_moxCv;
  std::thread m_moxTimerThread;
  bool m_moxActive = false;
  bool m_stopMoxTimerThread = false;
  bool m_moxDeactivationScheduled = false;
  uint64_t m_moxScheduleToken = 0;
  uint64_t m_moxDeactivationAtMs = 0;
  int m_moxReleaseDelayMs = 250;
};

#endif //CWKEYERAPP_REMOTECLIENT_H


#ifndef CWKEYERAPP_REMOTECLIENT_H
#define CWKEYERAPP_REMOTECLIENT_H

#include "../utils/IKeyerCW.h"
#include "../utils/Logger.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

/// Represents a single CW element to be sent to the remote server: the keyed
/// duration and the trailing space (inter-element / letter / word) duration.
struct CwElement {
  int duration = 0;
  int spaceDuration = 0;
};

/**
 * WebSocket client that keys a remote transceiver.
 *
 * Common logic (WebSocket framing, handshake and MOX management)
 * lives in RemoteClient.cpp. Platform-specific socket primitives are
 * implemented in RemoteClient_win.cpp / RemoteClient_linux.cpp.
 */
class RemoteClient : public IKeyerCW {
public:
  RemoteClient();
  ~RemoteClient() override;

  /// Connects to the server, performs the WebSocket handshake and starts the MOX timer thread.
  bool start(const std::string &serverIp, int port, int moxReleaseDelayMs);
  /// Releases MOX, stops the timer thread and closes the connection.
  void stop();
  /// Returns true while the client is connected and operational.
  bool started() const;

  /// Enqueues a timed CW element (dit/dah) to be sent to the remote server.
  void runCW(KeyerItem item, int duration, int spaceDuration) override;
  /// Presses the remote key (key down).
  void startRunCw() override;
  /// Releases the remote key (key up).
  void stopRunCw() override;

private:
  // --- Common logic (RemoteClient.cpp) ---
  bool performWebSocketHandshake(const std::string &serverIp, int port);
  void moxTimerLoop();
  void senderLoop();
  bool sendCommand(bool keyDown);
  bool sendKeyerCommand(bool keyDown, int intervalMs);
  bool sendFrame(uint8_t opcode, const uint8_t *payload, size_t payloadLen);
  bool sendControlFrame(uint8_t opcode, const std::vector<uint8_t> &payload);
  bool sendLine(const std::string &line);
  int readExact(uint8_t *buffer, size_t len, int timeoutMs);
  int recvFrame(uint8_t &opcode, std::vector<uint8_t> &payload, int timeoutMs);
  void webSocketLoop();

  // --- Platform-specific primitives (RemoteClient_win.cpp / RemoteClient_linux.cpp) ---
  /// Performs platform network initialization (e.g. WSAStartup on Windows).
  bool platformInit();
  /// Performs platform network cleanup (e.g. WSACleanup on Windows).
  void platformCleanup();
  /// Opens a TCP connection to ip:port; stores the descriptor in m_socketFd.
  bool openConnection(const std::string &serverIp, int port);
  /// Closes the socket stored in m_socketFd, if open.
  void closeConnection();
  /// Sends the full buffer over the socket; returns false on any error.
  bool sendRaw(const void *data, size_t len);
  /// Receives up to len bytes; waits at most timeoutMs (-1 = blocking). Returns bytes read, -2 on timeout, 0 on close, -1 on error.
  int recvRaw(void *buffer, size_t len, int timeoutMs);

  intptr_t m_socketFd = -1;
  std::atomic<bool> m_running{false};
  std::mutex m_sendMutex;
  std::thread m_webSocketThread;
  std::thread m_senderThread;
  std::atomic<bool> m_stopWebSocketThread{false};
  std::atomic<bool> m_stopSenderThread{false};
  std::mutex m_queueMutex;
  std::condition_variable m_queueCv;
  std::queue<CwElement> m_cwQueue;
  uint64_t m_lastSendKeyerCommandAtMs = 0;


#ifdef _WIN32
  bool m_wsaStarted = false;
#endif
};

#endif //CWKEYERAPP_REMOTECLIENT_H

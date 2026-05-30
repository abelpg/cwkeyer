#ifndef CWKEYERAPP_LOGGER_H
#define CWKEYERAPP_LOGGER_H


#include <iostream>
#include <sstream>
enum LogLevel {
  L_ERROR, L_WARNING, L_INFO, L_DEBUG
};


class Logger {
public:
  Logger(LogLevel logLevel = L_ERROR) {
    m_buffer << logLevel << " :"
        << std::string(
            logLevel > L_DEBUG
            ? (logLevel - L_DEBUG) * 4
            : 1
            , ' ');
  }

  template <typename T>
  Logger & operator<<(T const & value)  {
    m_buffer << value;
    return *this;
  }

  ~Logger()  {
    m_buffer << std::endl;
    // This is atomic according to the POSIX standard
    // http://www.gnu.org/s/libc/manual/html_node/Streams-and-Threads.html
    std::cerr << m_buffer.str();
  }

private:
  std::ostringstream m_buffer;
};

extern LogLevel loglevel;

#define log(level) \
if (level > loglevel) ; \
else Logger(level)

#endif //CWKEYERAPP_LOGGER_H

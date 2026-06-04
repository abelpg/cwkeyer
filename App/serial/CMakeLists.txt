cmake_minimum_required(VERSION 3.16)

add_library(LibSerial
        SerialComm.cpp
        SerialComm.h
        SerialPorts.cpp
        SerialPorts.h
        N1MMProxy.cpp
        N1MMProxy.h
)

target_include_directories(LibSerial
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Windows: link advapi32 for Registry APIs
if(WIN32)
    target_link_libraries(LibSerial PRIVATE advapi32)
endif()

# Linux: pthreads (std::thread), no extra libs for termios / ioctl
if(UNIX AND NOT APPLE)
    target_link_libraries(LibSerial PRIVATE pthread)
endif()


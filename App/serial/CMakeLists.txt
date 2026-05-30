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

cmake_minimum_required(VERSION 3.16)

add_library(LibSerial
        SerialComm.cpp
        SerialComm.h
)

target_include_directories(LibSerial
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(LibSerial PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::SerialPort
)
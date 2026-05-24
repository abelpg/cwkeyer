cmake_minimum_required(VERSION 3.16)

add_library(UsbKeyer
        UsbDevice.cpp
        UsbDevice.h
)
set(CMAKE_CXX_STANDARD 23)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb)
add_subdirectory(libusb)

target_include_directories(UsbKeyer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(UsbKeyer PRIVATE
        LibUsb
        Qt${QT_VERSION_MAJOR}::Core
)
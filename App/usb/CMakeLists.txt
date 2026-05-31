cmake_minimum_required(VERSION 3.16)

add_library(LibUsbDevice
        DeviceInterface.cpp
        DeviceInterface.h
        Device.cpp
        Device.h
        UsbDevice.cpp
        UsbDevice.h
)


target_include_directories(LibUsbDevice
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(LibUsbDevice PRIVATE
        LibConfiguration
        LibUtils
        ${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb-1.0.a
        Qt${QT_VERSION_MAJOR}::Core
)


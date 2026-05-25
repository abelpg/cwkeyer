cmake_minimum_required(VERSION 3.16)

add_library(UsbKeyer
        VendorProduct.cpp
        VendorProduct.h
        UsbDevice.cpp
        UsbDevice.h
        HidDevice.cpp
        HidDevice.h
)


target_include_directories(UsbKeyer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(UsbKeyer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb-1.0.a
        ${CMAKE_CURRENT_SOURCE_DIR}/hidapi/hidapi.dll
        Qt${QT_VERSION_MAJOR}::Core
)
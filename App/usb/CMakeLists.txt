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


# ── libusb: vendored on Windows, system package on Linux ─────────────────────
if(WIN32)
    target_link_libraries(LibUsbDevice PRIVATE
            LibConfiguration LibUtils
            ${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb-1.0.a
            Qt${QT_VERSION_MAJOR}::Core
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    target_include_directories(LibUsbDevice PRIVATE ${LIBUSB_INCLUDE_DIRS})
    target_link_libraries(LibUsbDevice PRIVATE
            LibConfiguration LibUtils
            ${LIBUSB_LIBRARIES}
            Qt${QT_VERSION_MAJOR}::Core
    )
endif()

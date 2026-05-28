cmake_minimum_required(VERSION 3.16)

add_library(GuiConnector
        GuiConnector.cpp
        GuiConnector.h
)


target_include_directories(GuiConnector
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(GuiConnector PRIVATE
        LibConfiguration
        LibSound
        LibSerial
        LibUsbDevice
        LibUtils
        KeyerLib
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Gui
)


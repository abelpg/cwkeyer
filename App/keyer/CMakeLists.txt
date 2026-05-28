cmake_minimum_required(VERSION 3.16)

add_library(KeyerLib
    Keyer.cpp
    Keyer.h
)


target_include_directories(KeyerLib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(KeyerLib PRIVATE
        LibUtils
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Gui
)


cmake_minimum_required(VERSION 3.16)

add_library(LibSound
        Sound.cpp
        Sound.h
)

target_include_directories(LibSound
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(LibSound PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Multimedia
)
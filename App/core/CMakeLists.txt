cmake_minimum_required(VERSION 3.16)

add_library(KeyerCore
        Sound.cpp
        Sound.h
)

target_include_directories(KeyerCore
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(KeyerCore PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Multimedia
)
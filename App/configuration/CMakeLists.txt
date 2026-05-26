cmake_minimum_required(VERSION 3.16)

add_library(LibConfiguration
        Configuration.cpp
        Configuration.h
        ConfigurationValue.cpp
        ConfigurationValue.h
)

target_include_directories(LibConfiguration
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LibConfiguration PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
)
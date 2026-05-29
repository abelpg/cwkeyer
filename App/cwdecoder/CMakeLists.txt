cmake_minimum_required(VERSION 3.16)

add_library(LibCwDecoder
        CwDecoder.cpp
        CwDecoder.h
)

target_include_directories(LibCwDecoder
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

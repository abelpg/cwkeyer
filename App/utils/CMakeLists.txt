cmake_minimum_required(VERSION 3.16)

add_library(LibUtils
        Utils.cpp
        Utils.h
        Logger.h
        IDitDah.h
        IKeyerCW.h
)

target_include_directories(LibUtils
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


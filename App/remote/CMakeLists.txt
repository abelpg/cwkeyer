cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(REMOTE_PLATFORM_SOURCES
            RemoteClient_win.cpp
    )
else()
    set(REMOTE_PLATFORM_SOURCES
            RemoteClient_linux.cpp
    )
endif()

add_library(LibRemote
        RemoteClient.cpp
        ${REMOTE_PLATFORM_SOURCES}
        RemoteClient.h
)

target_include_directories(LibRemote
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LibRemote PRIVATE
        LibUtils
)

if(WIN32)
    target_link_libraries(LibRemote PRIVATE ws2_32)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(LibRemote PRIVATE pthread)
endif()


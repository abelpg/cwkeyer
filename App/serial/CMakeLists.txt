cmake_minimum_required(VERSION 3.16)

# ── Platform-specific source selection ───────────────────────────────────────
if(WIN32)
    set(SERIAL_IMPL
            SerialComm_win.cpp
            SerialPorts_win.cpp
            N1MMProxy_win.cpp
    )
else()
    set(SERIAL_IMPL
            SerialComm_linux.cpp
            SerialPorts_linux.cpp
            N1MMProxy_linux.cpp
    )
endif()

add_library(LibSerial
        SerialComm.h
        SerialPorts.h
        N1MMProxy.h
        ${SERIAL_IMPL}
)

target_include_directories(LibSerial
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Windows: Registry APIs
if(WIN32)
    target_link_libraries(LibSerial PRIVATE advapi32)
endif()

# Linux: pthreads
if(UNIX AND NOT APPLE)
    target_link_libraries(LibSerial PRIVATE pthread)
endif()

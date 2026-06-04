cmake_minimum_required(VERSION 3.16)

add_library(LibKeyboard
        Keyboard.cpp
        Keyboard.h
        KeyboardListener.cpp
        KeyboardListener.h
)

target_include_directories(LibKeyboard
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LibKeyboard PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
)

# On Linux, uinput and evdev need no extra libs (kernel headers only),
# but we need pthreads (usually pulled in via Qt already).
if(UNIX AND NOT APPLE)
    target_link_libraries(LibKeyboard PRIVATE pthread)
endif()

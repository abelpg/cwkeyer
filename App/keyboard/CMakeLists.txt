cmake_minimum_required(VERSION 3.16)

# ── Platform-specific source selection ───────────────────────────────────────
if(WIN32)
    set(KEYBOARD_IMPL
            Keyboard_win.cpp
            KeyboardListener_win.cpp
    )
else()
    set(KEYBOARD_IMPL
            Keyboard_linux.cpp
            KeyboardListener_linux.cpp
    )
endif()

add_library(LibKeyboard
        Keyboard.h
        KeyboardListener.h
        ${KEYBOARD_IMPL}
)

target_include_directories(LibKeyboard
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LibKeyboard PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
)

if(UNIX AND NOT APPLE)
    target_link_libraries(LibKeyboard PRIVATE pthread)
endif()

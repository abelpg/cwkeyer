
target_sources(${CMAKE_PROJECT_NAME} PUBLIC
    main.cpp
)

add_subdirectory(core)
add_subdirectory(usb)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        KeyerCore
        UsbKeyer
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
)
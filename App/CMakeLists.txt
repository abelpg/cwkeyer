
target_sources(${CMAKE_PROJECT_NAME} PUBLIC
    main.cpp
)


add_subdirectory(utils)
add_subdirectory(configuration)
add_subdirectory(sound)
add_subdirectory(usb)



target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        LibConfiguration
        LibSound
        LibUsbDevice
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
)
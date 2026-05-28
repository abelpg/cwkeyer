
cmake_minimum_required(VERSION 3.21.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ON by default, we don't want to link the insight tracker library, as it is only used for development and testing purposes.
# This option allows developers to easily enable or disable linking the library without modifying the CMakeLists.txt file.
option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

project(CwKeyerApp LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)
set(QML_IMPORT_PATH ${QT_QML_OUTPUT_DIRECTORY}
    CACHE STRING "Import paths for Qt Creator's code model"
    FORCE
)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Widgets Qml Quick ShaderTools Multimedia)
qt_standard_project_setup()


qt_add_executable(${CMAKE_PROJECT_NAME})

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/App/usb/hidapi/hidapi.dll
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>")

qt_add_resources(${CMAKE_PROJECT_NAME} "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf)

include(qds)

if (BUILD_QDS_COMPONENTS)
    include(qmlcomponents OPTIONAL)
endif()

if (LINK_INSIGHT)
    include(insight OPTIONAL)
endif ()

include(GNUInstallDirs)
install(TARGETS ${CMAKE_PROJECT_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# En CMakeLists.txt raíz, añadir:
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)  # LTO

# Flags de optimización explícitos:
if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /O2 /GL)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE /LTCG)
else()
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O2 -flto)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -flto)
endif()

#windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw --qmldir .\qml CwKeyerApp.exe

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



if(WIN32)
    list(APPEND app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/CwKeyer.rc")
endif()

if(WIN32 AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))
    qt_add_executable(${CMAKE_PROJECT_NAME} WIN32 ${app_icon_resource_windows})
else()
    qt_add_executable(${CMAKE_PROJECT_NAME} ${app_icon_resource_windows})
endif()

#add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_CURRENT_SOURCE_DIR}/App/usb/hidapi/hidapi.dll
#        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>")

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
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)  # LTO
#
## Flags de optimización explícitos:
#if(MSVC)
#    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /O2 /GL)
#    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE /LTCG)
#else()
#    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O2 -flto)
#    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -flto)
#endif()

# ── CPack / Installer ──────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME              "CwKeyer")
set(CPACK_PACKAGE_VENDOR            "EA1FXG")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CW Keyer Application")
set(CPACK_PACKAGE_VERSION           "1.2.2-BETA")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CwKeyer")

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)   # crea carpeta raíz dentro del ZIP
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-portable")

# Icono / licencia (opcionales pero recomendados)
#set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# ── Windows: NSIS o WiX ───────────────────────────────────────────────────────
if(WIN32)
    # Elige uno: NSIS (requiere NSIS instalado) o WIX (requiere WiX Toolset)
    set(CPACK_GENERATOR "ZIP;NSIS")   # genera ambos: ZIP portable + instalador NSIS

    # NSIS — opciones extra
    set(CPACK_NSIS_DISPLAY_NAME     "CwKeyer")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH      ON)   # añade el binario al PATH

    # WiX — opciones extra (descomenta si usas WIX)
    # set(CPACK_WIX_UPGRADE_GUID   "GENERA-UN-GUID-UNICO-AQUI")
else()
    set(CPACK_GENERATOR "TGZ")        # Linux/Mac: tar.gz portable
endif()



# Instalar hidapi.dll junto al ejecutable (solo Windows)
if(WIN32)
install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/App/usb/libusb/libusb-1.0.dll"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
endif()

# Instalar DLLs de Qt (usando windeployqt o instalando manualmente)
# Opción automática con Qt helper (Qt 6.3+):
qt_generate_deploy_app_script(
        TARGET ${CMAKE_PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        DEPLOY_TOOL_OPTIONS "--qmldir" "${CMAKE_SOURCE_DIR}"  # apunta al directorio con tus .qml
)
install(SCRIPT ${deploy_script})


include(CPack)

#cmake -B cmake-build-debug
#cmake --build cmake-build-debug --config Release
#cmake --build cmake-build-debug -t clean
#cd cmake-build-debug
#cpack -G NSIS   # o WIX  cpack -G ZIP
#windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw --qmldir .\qml CwKeyerApp.exe
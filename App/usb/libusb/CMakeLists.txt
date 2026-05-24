cmake_minimum_required(VERSION 3.16)

message("-- Linking LibUsb")

# If CMAKE_C_CREATE_STATIC_LIBRARY is set it will override these.
if(NOT DEFINED CMAKE_C_ARCHIVE_CREATE)
    set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_C_ARCHIVE_APPEND)
    set(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_C_ARCHIVE_FINISH)
    set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
endif()
# compile a C file into an object file
if(NOT CMAKE_C_COMPILE_OBJECT)
    set(CMAKE_C_COMPILE_OBJECT
            "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
endif()

if(NOT CMAKE_C_LINK_EXECUTABLE)
    set(CMAKE_C_LINK_EXECUTABLE
            "<CMAKE_C_COMPILER> <FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()


add_library(LibUsb libusb-1.0.a)
set_target_properties(LibUsb PROPERTIES LINKER_LANGUAGE C)
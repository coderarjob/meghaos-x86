cmake_minimum_required(VERSION 3.10)
project(MeghaOS-V2 C ASM_NASM)

include(cmake/assemble.cmake)

set(MOS_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(MOS_LISTS_DIR ${CMAKE_BINARY_DIR}/lists)

file(MAKE_DIRECTORY ${MOS_BIN_DIR})
file(MAKE_DIRECTORY ${MOS_LISTS_DIR})

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(src/bootloader/x86)
else()
endif()

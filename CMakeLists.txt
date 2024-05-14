cmake_minimum_required(VERSION 3.10)
project(MeghaOS-V2 C ASM_NASM)

include(cmake/assemble.cmake)

set(MOS_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(MOS_LISTS_DIR ${CMAKE_BINARY_DIR}/lists)

file(MAKE_DIRECTORY ${MOS_BIN_DIR})
file(MAKE_DIRECTORY ${MOS_LISTS_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MOS_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(src/bootloader/x86)
    add_subdirectory(src/common)
    add_subdirectory(src/kernel)
    add_subdirectory(src/kernel/x86)
else()
    message(FATAL_ERROR "Not implemented")
endif()

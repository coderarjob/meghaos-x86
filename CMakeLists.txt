cmake_minimum_required(VERSION 3.10)
project(MeghaOS-V2 C ASM_NASM)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(src/bootloader/x86)
else()
endif()

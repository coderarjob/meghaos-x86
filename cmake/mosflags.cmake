if(CMAKE_CROSSCOMPILING)
    include(cmake/${ARCH}/kernelflags.cmake)
    include(cmake/${ARCH}/userflags.cmake)

    set(MOS_LINKER_OPTIONS
        -ffreestanding
        -nostdlib
        )

    set(MOS_ASM_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include/asm
        ${PROJECT_SOURCE_DIR}/include/x86/asm
        )

    set(MOS_GCC_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include
        )

    if (MOS_BUILD_MODE STREQUAL "DEBUG")
        list(APPEND MOS_KERNEL_GCC_FLAGS -g)
        list(APPEND MOS_KERNEL_NASM_ELF_MODE_FLAGS -g)

        list(APPEND MOS_USER_GCC_FLAGS -g)
        list(APPEND MOS_USER_NASM_ELF_MODE_FLAGS -g)
    endif()

    If (MOS_GRAPHICS_ENABLED)
        list(APPEND MOS_KERNEL_GCC_FLAGS -DGRAPHICS_MODE_ENABLED)
        list(APPEND MOS_KERNEL_NASM_ELF_MODE_FLAGS -DGRAPHICS_MODE_ENABLED)
        list(APPEND MOS_KERNEL_NASM_BIN_MODE_FLAGS -DGRAPHICS_MODE_ENABLED)

        list(APPEND MOS_USER_GCC_FLAGS -DGRAPHICS_MODE_ENABLED)
        list(APPEND MOS_USER_NASM_ELF_MODE_FLAGS -DGRAPHICS_MODE_ENABLED)
        list(APPEND MOS_USER_NASM_BIN_MODE_FLAGS -DGRAPHICS_MODE_ENABLED)

        list(APPEND MOS_KERNEL_GCC_FLAGS -DGRAPHICS_BPP=${MOS_GRAPHICS_BPP})
        list(APPEND MOS_KERNEL_NASM_BIN_MODE_FLAGS -DGRAPHICS_BPP=${MOS_GRAPHICS_BPP})
        list(APPEND MOS_KERNEL_NASM_ELF_MODE_FLAGS -DGRAPHICS_BPP=${MOS_GRAPHICS_BPP})

        list(APPEND MOS_USER_GCC_FLAGS -DGRAPHICS_BPP=${MOS_GRAPHICS_BPP})
        list(APPEND MOS_USER_NASM_BIN_MODE_FLAGS -DGRAPHICS_BPP=${MOS_GRAPHICS_BPP})
        list(APPEND MOS_USER_NASM_ELF_MODE_FLAGS -DGRAPHICS_BPP=${MOS_GRAPHICS_BPP})
    endif()

else()
    include(cmake/${ARCH}/unittestflags.cmake)
endif()

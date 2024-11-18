if(CMAKE_CROSSCOMPILING)
    include(cmake/${ARCH}/kernelflags.cmake)
    include(cmake/${ARCH}/userflags.cmake)

    set(MOS_LINKER_OPTIONS
        -ffreestanding
        -nostdlib
        )

    if (MOS_BUILD_MODE STREQUAL "DEBUG")
        list(APPEND MOS_KERNEL_GCC_FLAGS -g)
        list(APPEND MOS_KERNEL_NASM_ELF_MODE_FLAGS -g)

        list(APPEND MOS_USER_GCC_FLAGS -g)
    endif()

    If (MOS_GRAPHICS_ENABLED)
        list(APPEND MOS_KERNEL_GCC_DEFINITIONS
            GRAPHICS_MODE_ENABLED
            GRAPHICS_BPP=${MOS_GRAPHICS_BPP}
        )

        list(APPEND MOS_KERNEL_NASM_DEFINITIONS
            GRAPHICS_MODE_ENABLED
            GRAPHICS_BPP=${MOS_GRAPHICS_BPP}
        )

        list(APPEND MOS_USER_GCC_DEFINITIONS
            GRAPHICS_MODE_ENABLED
            GRAPHICS_BPP=${MOS_GRAPHICS_BPP}
        )
    endif()

else()
    include(cmake/${ARCH}/unittestflags.cmake)
endif()

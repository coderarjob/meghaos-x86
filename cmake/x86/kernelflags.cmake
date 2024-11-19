include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/commonflags.cmake)

# ----------------------------------------------------
# GCC
# ----------------------------------------------------
set(MOS_KERNEL_GCC_FLAGS
    ${MOS_GCC_WARN_FLAGS}
    ${MOS_GCC_FLAGS}
)

set(MOS_KERNEL_GCC_DEFINITIONS
    ${MOS_BUILD_MODE}
    MARCH=${MARCH}
    ARCH=${ARCH}
    KERNEL
)

if (MOS_PORT_E9_ENABLED)
    list(APPEND MOS_KERNEL_GCC_DEFINITIONS PORT_E9_ENABLED)
endif()

set(MOS_KERNEL_GCC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
)

# ----------------------------------------------------
# NASM
# ----------------------------------------------------
set(MOS_KERNEL_NASM_DEFINITIONS
    KERNEL
)

set(MOS_KERNEL_NASM_BIN_MODE_FLAGS
    -O0
    -f bin
    )

set(MOS_KERNEL_NASM_ELF_MODE_FLAGS
    -O0
    -f elf
   )

set(MOS_KERNEL_ASM_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/asm
    ${PROJECT_SOURCE_DIR}/include/x86/asm
)

# ----------------------------------------------------
# Both GCC & NASM
# ----------------------------------------------------
if (MOS_BUILD_MODE STREQUAL "DEBUG")
    list(APPEND MOS_KERNEL_GCC_FLAGS -g)
    list(APPEND MOS_KERNEL_NASM_ELF_MODE_FLAGS -g)
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
endif()


# ----------------------------------------------------
# LINKER
# ----------------------------------------------------
set(MOS_KERNEL_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/kernel.ld)

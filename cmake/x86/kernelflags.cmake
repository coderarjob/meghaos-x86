set(MOS_KERNEL_GCC_WARN_FLAGS
    -Wpedantic
    -Wall
    -Wextra
    -Wconversion
    -Wdangling-else
    -Werror
    )

set(MOS_KERNEL_GCC_FLAGS
    ${MOS_KERNEL_GCC_WARN_FLAGS}
    -std=c99
    -nostartfiles
    -ffreestanding
    -fno-pie
    -fno-stack-protector
    -fno-asynchronous-unwind-tables
    -m32
    -march=i686
    -masm=intel
    -mno-red-zone
    -mno-sse
    -malign-data=abi
    -Os
    -fno-unit-at-a-time
    -fno-omit-frame-pointer
    -fno-inline-functions-called-once
    -DDEBUG_LEVEL=${MOS_DEBUG_LEVEL}
    -D${MOS_BUILD_MODE}
    -DMARCH=${MARCH}
    -DARCH=${ARCH}
    )

set(MOS_KERNEL_NASM_BIN_MODE_FLAGS
    -O0
    -f bin
    )

set(MOS_KERNEL_NASM_ELF_MODE_FLAGS
    -O0
    -f elf
   )

if (MOS_BUILD_MODE STREQUAL "DEBUG")
    set(MOS_KERNEL_GCC_FLAGS ${MOS_KERNEL_GCC_FLAGS} -g)
    set(MOS_KERNEL_NASM_ELF_MODE_FLAGS ${MOS_KERNEL_NASM_ELF_MODE_FLAGS} -g)
endif()

If (MOS_GRAPHICS_ENABLED)
    set(MOS_KERNEL_GCC_FLAGS ${MOS_KERNEL_GCC_FLAGS} -DGRAPHICS_MODE_ENABLED)
    set(MOS_KERNEL_NASM_ELF_MODE_FLAGS ${MOS_KERNEL_NASM_ELF_MODE_FLAGS} -DGRAPHICS_MODE_ENABLED)
    set(MOS_KERNEL_NASM_BIN_MODE_FLAGS ${MOS_KERNEL_NASM_BIN_MODE_FLAGS} -DGRAPHICS_MODE_ENABLED)
endif()

set(MOS_KERNEL_LINKER_OPTIONS
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

set(MOS_KERNEL_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/kernel.ld)
set(MOS_PROCESS_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/process.ld)


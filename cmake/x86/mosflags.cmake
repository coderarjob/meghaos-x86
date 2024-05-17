set(MOS_GCC_WARN_FLAGS
    -Wpedantic
    -Wall
    -Wextra
    -Wconversion
    -Wdangling-else
    -Werror
    )

set(MOS_GCC_FLAGS
    ${MOS_GCC_WARN_FLAGS}
    -std=c99
    -nostartfiles
    -ffreestanding
    -fno-pie
    -fno-stack-protector
    -fno-asynchronous-unwind-tables
    -m32
    -march=i386
    -masm=intel
    -mno-red-zone
    -mno-sse
    -O1
    -fno-unit-at-a-time
    -fno-omit-frame-pointer
    -fno-inline-functions-called-once
    -DDEBUG_LEVEL=${MOS_DEBUG_LEVEL}
    -D${MOS_BUILD_MODE}
    )

if (MOS_BUILD_MODE STREQUAL "Debug")
    set(MOS_GCC_FLAGS ${MOS_GCC_FLAGS} -g)
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

set(MOS_KERNEL_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/build/kernel.ld)

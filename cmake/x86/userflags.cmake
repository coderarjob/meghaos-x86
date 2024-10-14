set(MOS_USER_GCC_FLAGS
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
    -fno-inline-small-functions
    -D${MOS_BUILD_MODE}
    -DMARCH=${MARCH}
    -DARCH=${ARCH}
    )

set(MOS_USER_NASM_BIN_MODE_FLAGS
    -O0
    -f bin
    )

set(MOS_USER_NASM_ELF_MODE_FLAGS
    -O0
    -f elf
    )

set(MOS_USER_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/process.ld)

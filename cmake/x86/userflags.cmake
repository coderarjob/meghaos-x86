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
    )

set(MOS_USER_GCC_DEFINITIONS
    ${MOS_BUILD_MODE}
    MARCH=${MARCH}
    ARCH=${ARCH}
)

set(MOS_USER_GCC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/applib
)


set(MOS_USER_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/process.ld)

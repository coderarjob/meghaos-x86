set(MOS_GCC_WARN_FLAGS
    -Wpedantic
    -Wall
    -Wextra
    -Wconversion
    -Wdangling-else
    -Werror
    )

set(MOS_GCC_FLAGS
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
)

set(MOS_LINKER_OPTIONS
    -ffreestanding
    -nostdlib
)

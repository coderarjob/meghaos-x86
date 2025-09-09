#!/bin/bash

ROOT=../../../
CC=~/.local/opt/i686-cross/bin/i686-elf-gcc
BIN_DIR=$ROOT/build-os/bin/

GCC_KERNEL_INCLUDE_DIR="$ROOT/include"

GCC_LIBCM_DEFINITIONS="-DLIBCM \
                       -DKERNEL \
                       -DDEBUG"

GCC_KERNEL_FLAGS="-std=c99 \
                  -nostartfiles \
                  -ffreestanding \
                  -fno-pie \
                  -fno-stack-protector \
                  -fno-asynchronous-unwind-tables \
                  -m32 \
                  -march=i686 \
                  -masm=intel \
                  -mno-red-zone \
                  -mno-sse \
                  -malign-data=abi \
                  -Os \
                  -fno-unit-at-a-time \
                  -fno-omit-frame-pointer \
                  -fno-inline-functions-called-once \
                  -Wpedantic \
                  -Wall \
                  -Wextra \
                  -Wconversion \
                  -Wdangling-else \
                  -Werror"


$CC $GCC_KERNEL_FLAGS -I $GCC_KERNEL_INCLUDE_DIR $GCC_LIBCM_DEFINITIONS \
    -c $ROOT/src/cm/x86/crta.c -o crta.o || exit

zig build --prominent-compile-errors -Doptimize=ReleaseSafe || exit

objcopy -O binary ./zig-out/bin/hello hello.flt || exit

cp hello.flt $BIN_DIR || exit

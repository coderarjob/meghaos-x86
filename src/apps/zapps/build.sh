#!/bin/bash

ROOT=../../../
CC=~/.local/opt/i686-cross/bin/i686-elf-gcc
ZIG=zig

PROCESS_LD_SCRIPT=$ROOT/src/kernel/x86/process.ld
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

ZIG_FLAGS="-target x86-freestanding-none  \
           -fno-PIE \
           -fno-stack-protector \
           -fno-omit-frame-pointer \
           -mno-red-zone \
           -fno-unwind-tables \
           -mcpu=i686 \
           --no-eh-frame-hdr \
           -O ReleaseSafe"

ZIG_PROCESS_INCLUDE_DIR="$ROOT/include/cm"

ZIG_KERNEL_LD_FLAGS="--target=x86-freestanding-none \
                     -ffreestanding \
                     -nostdinc \
                     -mcpu=i686 \
                     -Wl,--entry,proc_start,--gc-sections,--no-eh-frame-hdr"

CM_LIB_FILE=$ROOT/build-os/src/cm/libcm.a

$ZIG build-obj hello.zig $ZIG_FLAGS -I $ZIG_PROCESS_INCLUDE_DIR --verbose-cc || exit

$CC $GCC_KERNEL_FLAGS -I $GCC_KERNEL_INCLUDE_DIR $GCC_LIBCM_DEFINITIONS \
    -c $ROOT/src/cm/x86/crta.c -o crta.o || exit

$ZIG cc $ZIG_KERNEL_LD_FLAGS -T $PROCESS_LD_SCRIPT crta.o hello.o $CM_LIB_FILE \
    -o hello.elf -v || exit

objcopy -O binary hello.elf hello.flt || exit

cp hello.flt $BIN_DIR || exit

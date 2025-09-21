#!/bin/bash

ROOT=../../../
CC=~/.local/opt/i686-cross/bin/i686-elf-gcc
BIN_DIR=$ROOT/build-os/bin/

PROCESS_LINKER_SCRIPT=$ROOT/src/kernel/x86/process.ld
LIBCM_PATH=$ROOT/build-os/src/cm
LIBCRT_PATH=$ROOT/build-os/src/cm/CMakeFiles/crta.dir/x86/crta.c.obj
PROCESS_INCLUDE_PATH=$ROOT/include/cm

zig build --prominent-compile-errors -Doptimize=ReleaseSafe \
    -DLinkerScriptPath=$PROCESS_LINKER_SCRIPT               \
    -DLibCMPath=$LIBCM_PATH                                 \
    -DCRTPath=$LIBCRT_PATH                                  \
    -DCInludePath=$PROCESS_INCLUDE_PATH || exit

cp zig-out/hello.flt $BIN_DIR || exit

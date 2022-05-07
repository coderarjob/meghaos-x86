#!/bin/bash

if [ $# -gt 0 ] && [ "$1" = "unittests" ]; then
    for fn in build/bin/unittests/*; do
        echo -e -n "==================================\n"
        echo -e -n "Test: $fn\n"
        echo -e -n "----------------------------------\n"
        ./$fn
        echo -e -n "\n"
    done
    exit 0
fi

OPTS="$@"
RAMMB=5
qemu-system-i386 $OPTS -fda ./build/diskimage/x86/mos.flp \
                         -boot a                          \
                         -m $RAMMB                        \
                         -cpu 486                         \
                         -debugcon stdio                  \
                         -no-reboot                       \
                         -no-shutdown                     \
                         -d cpu_reset
                 

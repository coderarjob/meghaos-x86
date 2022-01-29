#!/bin/sh

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
                 

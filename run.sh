#!/bin/sh

qemu-system-i386 -fda ./build/diskimage/x86/mos.flp \
                 -boot a                            \
                 -m 05                              \
                 -cpu 486                           \
                 -debugcon stdio                    \
                 -no-reboot                         \
                 -no-shutdown                       \
                 -d cpu_reset
                 

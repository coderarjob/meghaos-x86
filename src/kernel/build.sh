#!/bin/bash

# Builds Kernel (Platform dependent and independent parts)
export K_OBJDIR="$OBJDIR/kernel"

#build path
if [ -e $K_OBJDIR ]; then
    rm $K_OBJDIR/* || exit
else
    mkdir $K_OBJDIR || exit
fi

# Build the Platform dependent Kernel
bash src/kernel/x86/build.sh || exit

# Build the Kernel
echo "    [ Compiling Kernel: Platform independent parts ]    "

$GCC32 -c src/kernel/printk.c -o $K_OBJDIR/printk.o                    || exit
$GCC32 -S src/kernel/printk.c -o $LISTDIR/printk.lst   >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/mem.c -o $K_OBJDIR/mem.o                          || exit
$GCC32 -S src/kernel/mem.c -o $LISTDIR/mem.lst         >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/kpanic.c -o $K_OBJDIR/kpanic.o                    || exit
$GCC32 -S src/kernel/kpanic.c -o $LISTDIR/kpanic.lst   >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/errno.c -o $K_OBJDIR/errno.o                      || exit
$GCC32 -S src/kernel/errno.c -o $LISTDIR/errno.lst     >/dev/null 2>&1 || exit

$LD_KERNEL $LD_FLAGS $K_OBJDIR/*.o $LD_OPTIONS -o $OBJDIR/kernel.elf   || exit
$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt               || exit

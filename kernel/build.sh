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
bash kernel/x86/build.sh || exit

# Build the Kernel
echo "    [ Compiling Kernel: Platform independent parts ]    "

$GCC32 -c kernel/printk.c -o $K_OBJDIR/printk.o  || exit
$GCC32 -S kernel/printk.c -o $LISTDIR/printk.lst || exit

$GCC32 -c kernel/mem.c -o $K_OBJDIR/mem.o  || exit
$GCC32 -S kernel/mem.c -o $LISTDIR/mem.lst || exit

$GCC32 -c kernel/kpanic.c -o $K_OBJDIR/kpanic.o  || exit
$GCC32 -S kernel/kpanic.c -o $LISTDIR/kpanic.lst || exit

$GCC32 -c kernel/errno.c -o $K_OBJDIR/errno.o  || exit
$GCC32 -S kernel/errno.c -o $LISTDIR/errno.lst || exit

$LD_KERNEL $K_OBJDIR/*.o -o $K_OBJDIR/kernel.elf || exit
$OBJCOPY -O binary $K_OBJDIR/kernel.elf $OBJDIR/kernel.flt || exit

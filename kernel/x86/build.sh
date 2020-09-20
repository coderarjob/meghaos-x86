#!/bin/bash

# Builds Kernel
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the Kernel
echo "    [ Compilling Kernel ]    "
#nasm -f bin kernel/x86/kernel.s \
#   -I kernel/x86 $NASM_INCPATH \
#   -O0 -o $OBJDIR/kernel.flt \
#   -l $LISTDIR/kernel.lst || exit

$GCC32 -c kernel/x86/kernel.c -o $OBJDIR/kernel.o
$GCC32 -S kernel/x86/kernel.c -o $LISTDIR/kernel.lst

$LD_KERNEL $OBJDIR/kernel.o -o $OBJDIR/kernel.elf
objcopy -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt

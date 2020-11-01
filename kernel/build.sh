#!/bin/bash

# Builds Kernel
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the Kernel
echo "    [ Compilling Common Kernel ]    "
#nasm -f elf32 kernel/x86/gdt_write.s \
#    -I kernel/x86 $NASM_INCPATH \
#    -O0 -o $OBJDIR/gdt_write.o \
#    -l $LISTDIR/gdt_write.lst || exit

i686_GCC="$GCC32" 

$i686_GCC -c kernel/printk.c -o $OBJDIR/printk.o  || exit
$i686_GCC -S kernel/printk.c -o $LISTDIR/printk.lst || exit

$LD_KERNEL $OBJDIR/kernel_x86.o \
           $OBJDIR/printk.o \
           -o $OBJDIR/kernel.elf

$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt

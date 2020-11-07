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

$i686_GCC -c kernel/mem.c -o $OBJDIR/mem.o  || exit
$i686_GCC -S kernel/mem.c -o $LISTDIR/mem.lst || exit

$i686_GCC -c kernel/kpanic.c -o $OBJDIR/kpanic.o  || exit
$i686_GCC -S kernel/kpanic.c -o $LISTDIR/kpanic.lst || exit

$LD_KERNEL $OBJDIR/kernel_x86.o \
           $OBJDIR/printk.o \
           $OBJDIR/mem.o \
           $OBJDIR/kpanic.o \
           -o $OBJDIR/kernel.elf

$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt

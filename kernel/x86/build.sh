#!/bin/bash

# Builds Kernel
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the Kernel
echo "    [ Compilling Kernel ]    "
#nasm -f elf32 kernel/x86/gdt_write.s \
#    -I kernel/x86 $NASM_INCPATH \
#    -O0 -o $OBJDIR/gdt_write.o \
#    -l $LISTDIR/gdt_write.lst || exit

i686_GCC="$GCC32" 

$i686_GCC -c kernel/x86/kernel.c -o $OBJDIR/kernel.o 
$i686_GCC -S kernel/x86/kernel.c -o $LISTDIR/kernel.lst 

$i686_GCC -c kernel/x86/screen.c -o $OBJDIR/screen.o 
$i686_GCC -S kernel/x86/screen.c -o $LISTDIR/screen.lst

$i686_GCC -c kernel/x86/gdt.c -o $OBJDIR/gdt.o 
$i686_GCC -S kernel/x86/gdt.c -o $LISTDIR/gdt.lst

$LD_KERNEL $OBJDIR/kernel.o \
           $OBJDIR/screen.o \
           $OBJDIR/gdt.o \
           -o $OBJDIR/kernel.elf

$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt

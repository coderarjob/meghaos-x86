#!/bin/bash

# Builds Kernel
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the x86 part Kernel
# Later on kernel_x86.o will be linked with the common part into a elf binary.
echo "    [ Compilling x86 Kernel ]    "

i686_GCC="$GCC32" 

$i686_GCC -c kernel/x86/kernel.c -o $OBJDIR/kernel.o  || exit
$i686_GCC -S kernel/x86/kernel.c -o $LISTDIR/kernel.lst  || exit

$i686_GCC -c kernel/x86/vgadisp.c -o $OBJDIR/screen.o  || exit
$i686_GCC -S kernel/x86/vgadisp.c -o $LISTDIR/screen.lst || exit

$i686_GCC -c kernel/x86/gdt.c -o $OBJDIR/gdt.o  || exit
$i686_GCC -S kernel/x86/gdt.c -o $LISTDIR/gdt.lst || exit


$LD_KERNEL -relocatable \
           $OBJDIR/kernel.o \
           $OBJDIR/screen.o \
           $OBJDIR/gdt.o \
           -o $OBJDIR/kernel_x86.o


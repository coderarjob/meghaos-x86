#!/bin/bash

# Builds Kernel
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the x86 part Kernel
# Later on kernel_x86.o will be linked with the common part into a elf binary.
echo "    [ Compilling x86 Kernel ]    "

i686_GCC="$GCC32" 

nasm -f elf32 kernel/x86/usermode.s -g \
     -l $LISTDIR/x86_usermode.lst \
     -o $OBJDIR/x86_usermode.o  || exit

$i686_GCC -c kernel/x86/kernel.c -o $OBJDIR/x86_kernel.o  || exit
$i686_GCC -S kernel/x86/kernel.c -o $LISTDIR/x86_kernel.lst  || exit

$i686_GCC -c kernel/x86/vgadisp.c -o $OBJDIR/x86_screen.o  || exit
$i686_GCC -S kernel/x86/vgadisp.c -o $LISTDIR/x86_screen.lst || exit

$i686_GCC -c kernel/x86/tss.c -o $OBJDIR/x86_tss.o  || exit
$i686_GCC -S kernel/x86/tss.c -o $LISTDIR/x86_tss.lst || exit

$i686_GCC -c kernel/x86/gdt.c -o $OBJDIR/x86_gdt.o  || exit
$i686_GCC -S kernel/x86/gdt.c -o $LISTDIR/x86_gdt.lst || exit

$LD_KERNEL -relocatable \
           $OBJDIR/x86_kernel.o \
           $OBJDIR/x86_screen.o \
           $OBJDIR/x86_gdt.o \
           $OBJDIR/x86_usermode.o \
           $OBJDIR/x86_tss.o \
           -o $OBJDIR/kernel_x86.o || exit


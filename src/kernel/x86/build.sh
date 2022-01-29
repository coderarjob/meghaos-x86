#!/bin/bash

# Builds Kernel
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the x86 part Kernel
# Later on kernel_x86.o will be linked with the common part into a elf binary.
echo "    [ Compilling x86 Kernel ]    "

nasm -f elf32 src/kernel/x86/usermode.s -g \
     -l $LISTDIR/x86_usermode.lst \
     -o $K_OBJDIR/x86_usermode.o                                             || exit

nasm -f elf32 src/kernel/x86/entry.s -g \
     -l $LISTDIR/x86_entry.lst \
     -o $K_OBJDIR/x86_entry.o                                                || exit

# Kernel must must be named kernel.o, as this is hardcoded in kernel.ld
# NOTE: Must be a way to pass the filename

$GCC32 -c src/kernel/x86/kernel.c -o $K_OBJDIR/x86_kernel.o                   || exit
$GCC32 -S src/kernel/x86/kernel.c -o $LISTDIR/x86_kernel.lst  >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/x86/vgadisp.c -o $K_OBJDIR/x86_screen.o                  || exit
$GCC32 -S src/kernel/x86/vgadisp.c -o $LISTDIR/x86_screen.lst >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/x86/tss.c -o $K_OBJDIR/x86_tss.o                         || exit
$GCC32 -S src/kernel/x86/tss.c -o $LISTDIR/x86_tss.lst        >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/x86/gdt.c -o $K_OBJDIR/x86_gdt.o                         || exit
$GCC32 -S src/kernel/x86/gdt.c -o $LISTDIR/x86_gdt.lst        >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/x86/idt.c -o $K_OBJDIR/x86_idt.o                         || exit
$GCC32 -S src/kernel/x86/idt.c -o $LISTDIR/x86_idt.lst        >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/x86/kdebug.c -o $K_OBJDIR/x86_kdebug.o                   || exit
$GCC32 -S src/kernel/x86/kdebug.c -o $LISTDIR/x86_kdebug.lst  >/dev/null 2>&1 || exit

$GCC32 -c src/kernel/x86/paging.c -o $K_OBJDIR/x86_paging.o                   || exit
$GCC32 -S src/kernel/x86/paging.c -o $LISTDIR/x86_paging.lst  >/dev/null 2>&1 || exit

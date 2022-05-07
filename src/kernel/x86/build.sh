#!/bin/bash
# Builds Kernel (Platform dependent parts:  x86 architecture)

source functions.sh

# Build the x86 part Kernel
echo "    [ Compilling x86 Kernel ]    "

ASM_FILES=(
    'kernel/x86/usermode.s'
    'kernel/x86/entry.s'
)

C_FILES=(
    'kernel/x86/kernel.c'
    'kernel/x86/vgadisp.c'
    'kernel/x86/tss.c'
    'kernel/x86/gdt.c'
    'kernel/x86/idt.c'
    'kernel/x86/kdebug.c'
    'kernel/x86/paging.c'
)

compile_nasm "$NASM32_ELF" "$OBJDIR" ${ASM_FILES[@]}
compile_cc "$GCC32" "$OBJDIR" ${C_FILES[@]}

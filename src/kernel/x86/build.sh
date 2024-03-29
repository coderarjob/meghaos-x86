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
    'kernel/x86/pmm.c'
    'kernel/x86/boot.c'
    'kernel/x86/memmanage.c'
)

C_INTERRUPT_HANDLER_FILES=(
    'kernel/x86/interrupts.c'
)

compile_nasm "$NASM32_ELF" "$OBJDIR" ${ASM_FILES[@]}
compile_cc "$GCC32 $GCC32_FLAGS" "$OBJDIR" ${C_FILES[@]}
compile_cc "$GCC32 $GCC32_INTERRUPT_HANDLER_FLAGS" \
           "$OBJDIR"                               \
           ${C_INTERRUPT_HANDLER_FILES[@]}

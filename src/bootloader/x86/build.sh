#!/bin/bash

# Builds boot0 and boot1 bootloaders
# The procedures here are for the x86 architecture. Later on if I see that for
# all the other supported architecture, minor changes are required, then this
# same script will build all the others. 

# Build the bootloader 0
echo "    [ Compilling boot0 ]    "
nasm -f bin src/bootloader/x86/boot0/boot.s \
    -I src/bootloader/x86/boot0 $NASM_INCPATH \
    -O0 -o $OBJDIR/boot0.flt \
    -l $LISTDIR/boot0.lst || exit

# Build the bootloader 1
echo "    [ Compilling boot1 ]    "
nasm -f bin src/bootloader/x86/boot1/boot1.s \
     -I src/bootloader/x86/boot1 $NASM_INCPATH \
     -O0 -o $OBJDIR/boot1.flt \
     -l $LISTDIR/boot1.lst || exit

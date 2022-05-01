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
     -o $K_OBJDIR/x86_usermode.o                                        || exit

nasm -f elf32 src/kernel/x86/entry.s -g \
     -l $LISTDIR/x86_entry.lst \
     -o $K_OBJDIR/x86_entry.o                                           || exit

# Kernel must must be named kernel.o, as this is hardcoded in kernel.ld
# NOTE: Must be a way to pass the filename
FILES=('kernel.c'
       'vgadisp.c'
       'tss.c'
       'gdt.c'
       'idt.c'
       'kdebug.c'
       'paging.c'
)

for fn in ${FILES[@]}
do
    FILETILE=${fn%.*}
    FILEPATH=src/kernel/x86/$fn
    $GCC32 -c "$FILEPATH" -o $K_OBJDIR/x86_$FILETILE.o                  || exit
    $GCC32 -S "$FILEPATH" -o $LISTDIR/x86_$FILETILE.lst >/dev/null 2>&1 || exit
done

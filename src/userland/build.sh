#!/bin/bash
# Builds Kernel (Platform dependent parts:  x86 architecture)

source functions.sh

OBJDIR_PROCESS=$OBJDIR/userland/

# Build the x86 part Kernel
echo "    [ Compilling user processes]    "

# ---------------------------------------------------------------------------
# Build the floppy image
# ---------------------------------------------------------------------------
PROG_NAME="proc1"
C_FILES=(
    'userland/process1.c'
)

compile_cc "$GCC32 $GCC32_FLAGS" "$OBJDIR" ${C_FILES[@]}

OBJ_FILES=`find $OBJDIR/userland -name "*.o"`
$LD_KERNEL $LD_FLAGS_USERLAND     \
           $OBJ_FILES $LD_OPTIONS \
           -o $OBJDIR_PROCESS/$PROG_NAME.elf                           || exit
$OBJCOPY -O binary $OBJDIR_PROCESS/$PROG_NAME.elf \
         $OBJDIR_PROCESS/$PROG_NAME.flt                                || exit
# ---------------------------------------------------------------------------

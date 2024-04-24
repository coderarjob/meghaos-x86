#!/bin/bash
# Builds Kernel (Platform dependent parts:  x86 architecture)

source functions.sh

OBJDIR_PROCESS=$OBJDIR/userland/

# Build the x86 part Kernel
echo "    [ Compilling user processes]    "

# ---------------------------------------------------------------------------
# Program - Proc1
# ---------------------------------------------------------------------------
PROG_NAME="proc1"
echo "    [ Compilling $PROG_NAME program ]    "

C_FILES=(
    'userland/proc1.c'
)

compile_cc "$GCC32 $GCC32_FLAGS" "$OBJDIR" ${C_FILES[@]}

OBJ_FILES=`find $OBJDIR/userland -name "$PROG_NAME.o"`
$LD_KERNEL $LD_FLAGS_USERLAND     \
           $OBJ_FILES $LD_OPTIONS \
           -o $OBJDIR_PROCESS/$PROG_NAME.elf                           || exit
$OBJCOPY -O binary $OBJDIR_PROCESS/$PROG_NAME.elf \
         $OBJDIR_PROCESS/$PROG_NAME.flt                                || exit
# ---------------------------------------------------------------------------
# Program - mpdemo 
# ---------------------------------------------------------------------------
PROG_NAME="mpdemo"
echo "    [ Compilling $PROG_NAME program ]    "
C_FILES=(
    'userland/mpdemo.c'
)

compile_cc "$GCC32 $GCC32_FLAGS" "$OBJDIR" ${C_FILES[@]}

OBJ_FILES=`find $OBJDIR/userland -name "$PROG_NAME.o"`
$LD_KERNEL $LD_FLAGS_USERLAND     \
           $OBJ_FILES $LD_OPTIONS \
           -o $OBJDIR_PROCESS/$PROG_NAME.elf                           || exit
$OBJCOPY -O binary $OBJDIR_PROCESS/$PROG_NAME.elf \
         $OBJDIR_PROCESS/$PROG_NAME.flt                                || exit
# ---------------------------------------------------------------------------

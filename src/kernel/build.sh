#!/bin/bash
# Builds Kernel (Platform independent parts)

source functions.sh

# Build the Platform dependent Kernel
bash src/kernel/x86/build.sh || exit

# Build the Kernel
echo "    [ Compiling Kernel: Platform independent parts ]    "

C_FILES=(
    'kernel/printk.c'
    'kernel/mem.c'
    'kernel/kpanic.c'
    'kernel/errno.c'
)

compile_cc "$GCC32" $OBJDIR ${C_FILES[@]}

# Link all the object files in the $OBJDIR/kernel tree.
OBJ_FILES=`find $OBJDIR/kernel -name "*.o"`
$LD_KERNEL $LD_FLAGS $OBJ_FILES $LD_OPTIONS -o $OBJDIR/kernel.elf      || exit
$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt               || exit

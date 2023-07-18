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
    'kernel/disp.c'
    'kernel/kerror.c'
    'kernel/pmm.c'
    'common/bitmap.c'
    'common/utils.c'
)

compile_cc "$GCC32 $GCC32_FLAGS" $OBJDIR ${C_FILES[@]}

# Link all the object files in the $OBJDIR/kernel tree.
OBJ_FILES=`find $OBJDIR/kernel $OBJDIR/common -name "*.o"`
$LD_KERNEL $LD_FLAGS $OBJ_FILES $LD_OPTIONS -o $OBJDIR/kernel.elf      || exit
$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt               || exit

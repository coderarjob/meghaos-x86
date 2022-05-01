#!/bin/bash

# Builds Kernel (Platform dependent and independent parts)
export K_OBJDIR="$OBJDIR/kernel"

#build path
if [ -e $K_OBJDIR ]; then
    rm $K_OBJDIR/* || exit
else
    mkdir $K_OBJDIR || exit
fi

# Build the Platform dependent Kernel
bash src/kernel/x86/build.sh || exit

# Build the Kernel
echo "    [ Compiling Kernel: Platform independent parts ]    "

FILES=('printk.c'
       'mem.c'
       'kpanic.c'
       'errno.c'
)

for fn in ${FILES[@]}
do
    FILETILE=${fn%.*}
    FILEPATH=src/kernel/$fn
    $GCC32 -c "$FILEPATH" -o $K_OBJDIR/$FILETILE.o                     || exit
    $GCC32 -S "$FILEPATH" -o $LISTDIR/$FILETILE.lst >/dev/null 2>&1    || exit
done

$LD_KERNEL $LD_FLAGS $K_OBJDIR/*.o $LD_OPTIONS -o $OBJDIR/kernel.elf   || exit
$OBJCOPY -O binary $OBJDIR/kernel.elf $OBJDIR/kernel.flt               || exit

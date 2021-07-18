#!/bin/bash

export ARCH=x86
export DEBUG=NDEBUG
LINK_USING_LD=1

if [ $# -ge 1 ]; then ARCH=$1; fi
if [ $# -ge 2 ]; then DEBUG=$2; fi

export TEMPDIR="build/temp"
export OBJDIR="build/obj"
export SYMDIR="build/sym"
export LISTDIR="build/list"
export IMAGEDIR="build/diskimage/x86"
export DISKTEMPDIR="build/diskimage/temp"

export NASM_INCPATH="-I include/x86/asm -I include/asm"
export GCC_INCPATH="-I include -I include/x86"

# IMPORTAINT NOTE:
# __main must not be reordered. It must reside at the entry address.
# Without this flag, boot1 may jump to a wrong function not __main.
# -fno-unit-at-a-time implies 
# -fno-toplevel-reorder and -fno-section-anchors. 
# -fno-toplevel-reorder prevents reordering of top level functions

# NOTE:
# If using GCC to compile assembly files in Intel syntax, use the following
# options: 
#           -Wa,--32,-msyntax=intel,-mnaked-reg

# NOTE:
# -masm=intel           : .intel_syntax attribute alternate in C files.
# -Wa,<assembler options>
# -Wa,--32              : assembler targets i386 CPU
# -Wa,msyntax=intel     : .intel_syntax attribute alternate in assembly files
# -Wa,mnaked-reg        : do not require % in front of registers

# NOTE:
# -nostartfiles includes -nostdlib, -nolibc, or -nodefaultlibs

export GCC32="i686-elf-gcc -std=c99\
              -g \
              -nostartfiles \
              -ffreestanding \
              -fno-pie \
              -fno-stack-protector \
              -fno-asynchronous-unwind-tables \
              -m32 \
              -march=i386 \
              -masm=intel \
              -mno-red-zone \
              -Wpedantic \
              -Wpadded \
              -Wextra \
              -Wall \
              $GCC_INCPATH \
              -O1 -fno-unit-at-a-time \
              -D $DEBUG"


# -libgcc is included because of helper functions used by gcc.
# For example: __udivdi3 function used for division of 64 bit integers.
# Note: LD_KERNEL LD_FLAGS {*.o files} LD_OPTIONS -o {output elf}. If
#       LD_OPTIONS were places before input files, the linking with -lgcc did 
#       not work.

# Link using the ld program. 
if [ $LINK_USING_LD -eq 1 ]; then
    GCCVER=$(gcc -v 2>&1|tail - -n 1|awk '{print $3}')
    LIBPATH=$(dirname $(readlink $(which i686-elf-ld)))/../lib/gcc/i686-elf

    export LD_OPTIONS="-lgcc"       
    export LD_FLAGS="--nmagic \
                     --script build/kernel.ld \
                     -L $LIBPATH/$GCCVER/"
    export LD_KERNEL="i686-elf-ld"
else
     export LD_OPTIONS="-ffreestanding \
                       -nostdlib \
                       -lgcc"       
     export LD_FLAGS="-T build/kernel.ld"
     export LD_KERNEL="i686-elf-gcc"
fi

export OBJCOPY="i686-elf-objcopy"

# Create folders
if [ ! -e $TEMPDIR ]; then
    mkdir $TEMPDIR || exit
fi

if [ ! -e $DISKTEMPDIR ]; then
    mkdir -p $DISKTEMPDIR || exit
fi

if [ ! -e $OBJDIR ]; then
    mkdir $OBJDIR || exit
fi

if [ ! -e $IMAGEDIR ]; then
    mkdir -p $IMAGEDIR || exit
fi

if [ ! -e $LISTDIR ]; then
    mkdir $LISTDIR || exit
fi

# Cleanup directories
rm -f $TEMPDIR/* || exit
rm -f $DISKTEMPDIR/* || exit
rm -fr $OBJDIR/* || exit
rm -f $IMAGEDIR/* || exit
rm -f $LISTDIR/* || exit

# Build the bootloaders
bash src/bootloader/x86/build.sh || exit

# Build kernel
bash src/kernel/build.sh || exit

# Build the floppy image
echo "    [ Creating disk image ]    "
mkdosfs -C $IMAGEDIR/mos.flp 1440 || exit

# mount the Disk image
echo "    [ Mounting Disk image ]    "
runas mount $IMAGEDIR/mos.flp $DISKTEMPDIR || exit

# Copy the files needed to the floppy
echo "    [ Copy files to floppy ]    "
runas cp -v $OBJDIR/boot1.flt $DISKTEMPDIR ||exit
runas cp -v $OBJDIR/kernel.flt $DISKTEMPDIR ||exit

# Unmount the image
echo "    [ Copy of files done. Unmounting image ]    "
runas umount $DISKTEMPDIR || exit

# Wrtie the bootloader
echo "    [ Writing bootloader to floppy image ]    "
dd conv=notrunc if=$OBJDIR/boot0.flt of=$IMAGEDIR/mos.flp || exit

echo "    [ Storage Utilization ]"
wc -c $OBJDIR/*.flt

echo "    [ Cleaning up ]"
rm -f -r $DISKTEMPDIR || exit

echo "    [ Generating tags file ]"
ctags -R . || exit

echo "    [ Done ]"

#!/bin/bash

ARCH=x86
if [ $# -ge 1 ]; then
    ARCH=$1
fi

export TEMPDIR="build/temp"
export OBJDIR="build/obj"
export SYMDIR="build/sym"
export LISTDIR="build/list"
export IMAGEDIR="build/diskimage/x86"
export DISKTEMPDIR="build/diskimage/temp"

export NASM_INCPATH="-I include/x86/asm -I include/asm"
export GCC_INCPATH="-I include -I include/x86"

# -fno-toplevel-reorder prevents reordering of top level functions
# -nostartfiles includes -nostdlib, -nolibc, or -nodefaultlibs

export GCC16="gcc -std=c99 \
                  -nostartfiles \
                  -c \
                  -gdwarf-2 \
                  -ffreestanding \
                  -fno-pie \
                  -fno-stack-protector \
                  -fno-asynchronous-unwind-tables \
                  -m16 \
                  -march=i386 \
                  -Wpedantic \
                  -Wextra \
                  -Wall \
                  $GCC_INCPATH \
                  -O0 "

export LD_LOADER="ld -m elf_i386 --nmagic --script=build/loader.ld"
# export LD_LOADER="ld -m elf_i386 --oformat binary --script=build/loader.ld"

# Create folders
if [ ! -e $TEMPDIR ]; then
    mkdir $TEMPDIR
fi

if [ ! -e $DISKTEMPDIR ]; then
    mkdir $DISKTEMPDIR
fi

if [ ! -e $OBJDIR ]; then
    mkdir $OBJDIR
fi

if [ ! -e $IMAGEDIR ]; then
    mkdir $IMAGEDIR
fi

if [ ! -e $LISTDIR ]; then
    mkdir $LISTDIR
fi

# Cleanup directories
rm -f $TEMPDIR/*
rm -f $DISKTEMPDIR/*
rm -f $OBJDIR/*
rm -f $IMAGEDIR/*
rm -f $LISTDIR/*

# Build the bootloaders
bash bootloader/x86/build.sh || exit

# Build the floppy image
echo "    [ Creating disk image ]    "
mkdosfs -C $IMAGEDIR/mos.flp 1440 || exit

# Wrtie the bootloader
echo "    [ Writing bootloader to floppy image ]    "
dd conv=notrunc if=$OBJDIR/boot0.flt of=$IMAGEDIR/mos.flp || exit

# mount the Disk image
echo "    [ Mounting Disk image ]    "
runas mount $IMAGEDIR/mos.flp $DISKTEMPDIR || exit

# Copy the files needed to the floppy
echo "    [ Copy files to floppy ]    "
runas cp -v $OBJDIR/boot1.flt $DISKTEMPDIR ||exit

# Unmount the image
echo "    [ Copy of files done. Unmounting image ]    "
runas umount $DISKTEMPDIR || exit

echo "    [ Storage Utilization ]"
wc -c $OBJDIR/*

echo "    [ Cleaning up ]"
rm -f -r $DISKTEMPDIR || exit

echo "    [ Done ]"

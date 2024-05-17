#!/bin/bash

# ---------------------------------------------------------------------------
# Usage:
# <path where flp files are stored> <destination path>
# ---------------------------------------------------------------------------
if [ $# -eq 0 ]; then
    echo "Usage $0 <directory to flp binary files> <destination path>"
    exit 1
fi

OBJDIR="$1"
IMAGEDIR="$2"
DISKTEMPDIR=$(mktemp -d)

# ---------------------------------------------------------------------------
# Build the floppy image
echo "    [ Creating disk image ]    "
rm -f $IMAGEDIR/mos.flp                 || exit
mkdosfs -C $IMAGEDIR/mos.flp -F 12 1440 || exit

# mount the Disk image
echo "    [ Mounting Disk image ]    "
sudo mount $IMAGEDIR/mos.flp $DISKTEMPDIR || exit
# ---------------------------------------------------------------------------
# Copy the files needed to the floppy
echo "    [ Copy files to floppy ]    "
sudo cp -v $OBJDIR/boot1.flt $DISKTEMPDIR ||exit
sudo cp -v $OBJDIR/kernel.flt $DISKTEMPDIR ||exit
#sudo cp -v $OBJDIR/userland/*.flt $DISKTEMPDIR ||exit

# Unmount the image
echo "    [ Copy of files done. Unmounting image ]    "
sudo umount $DISKTEMPDIR || exit

# Wrtie the bootloader
echo "    [ Writing bootloader to floppy image ]    "
dd conv=notrunc if=$OBJDIR/boot0.flt of=$IMAGEDIR/mos.flp || exit

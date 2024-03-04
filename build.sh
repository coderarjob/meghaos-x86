#!/bin/bash

# ---------------------------------------------------------------------------
# DEFAULT OPTIONS:
# ---------------------------------------------------------------------------
export ARCH=x86
export DEBUG=DEBUG
export LINK_USING_LD=1

# DEBUG LEVEL BITS
# x x x x x x [Screen] [E9]
export DEBUGLEVEL=3

# ---------------------------------------------------------------------------
# ARGUMENT PARSING
# ---------------------------------------------------------------------------
BUILDMODES=(KERNEL LINT ALL)

BUILDMODES_COUNT=${#BUILDMODES[@]}
for ((i=0; i < $BUILDMODES_COUNT; i++)); do
    mode=${BUILDMODES[i]}
    declare -ir $mode=$i
done

export BUILDMODE=KERNEL
if [ ! -z $1 ]; then
    case "$1" in
        --arch   ) ARCH=$2        ;;
        --release) DEBUG=NDEBUG   ;;
        --lint   ) BUILDMODE=LINT ;;
        --all    ) BUILDMODE=ALL  ;;
        *        )
            echo "Invalid command $@"
            exit 1
            ;;
    esac
fi

# ---------------------------------------------------------------------------
# DEFAULT PATHS:
# ---------------------------------------------------------------------------
export REPORTSDIR="build/reports"
export TEMPDIR="build/temp"
export OBJDIR="build/obj"
export SYMDIR="build/sym"
export LISTDIR="build/list"
export IMAGEDIR="build/diskimage/x86"
export DISKTEMPDIR="build/diskimage/temp"

export NASM_INCPATH="-I include/x86/asm -I include/asm"
export GCC_INCPATH="-I include"

# ---------------------------------------------------------------------------
# IMPORTAINT NOTES:
# ---------------------------------------------------------------------------
# __main must not be reordered. It must reside at the entry address.
# Without this flag, boot1 may jump to a wrong function not __main.
# -fno-unit-at-a-time implies 
# -fno-toplevel-reorder and -fno-section-anchors. 
# -fno-toplevel-reorder prevents reordering of top level functions
#
# Required for stack trace to work
#
# -fno-omit-frame-pointer : Sometime GCC skips saving EBP, and uses ESP only.
# -fno-inline-functions-called-once : Unnecessory inlining produces wrong stack
#                                     trace results.
# ---------------------------------------------------------------------------
# If using GCC to compile assembly files in Intel syntax, use the following
# options: 
#           -Wa,--32,-msyntax=intel,-mnaked-reg
# ---------------------------------------------------------------------------
# -masm=intel           : .intel_syntax attribute alternate in C files.
# -Wa,<assembler options>
# -Wa,--32              : assembler targets i386 CPU
# -Wa,msyntax=intel     : .intel_syntax attribute alternate in assembly files
# -Wa,mnaked-reg        : do not require % in front of registers
# ---------------------------------------------------------------------------
# -nostartfiles includes -nostdlib, -nolibc, or -nodefaultlibs
# ---------------------------------------------------------------------------
WOPTS="-Wpedantic \
       -Wall \
       -Wextra \
       -Wconversion \
       -Wdangling-else \
       -Werror"

export GCC32="i686-elf-gcc"
export GCC32_FLAGS="-std=c99\
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
                    $WOPTS \
                    $GCC_INCPATH \
                    -O1 \
                    -fno-unit-at-a-time \
                    -fno-omit-frame-pointer \
                    -fno-inline-functions-called-once \
                    -D $DEBUG -D DEBUG_LEVEL=$DEBUGLEVEL"

# Interrupt and exception handlers only preserve general purpose registers, this
# attribute prevents GCC to use any other registers (MMX, SSE etc) when
# compiling interrupt handlers.
export GCC32_INTERRUPT_HANDLER_FLAGS="$GCC32_FLAGS -mgeneral-regs-only"

export NASM32_ELF="nasm -f elf32 -g"
export NASM32_BIN="nasm -f bin"

# -libgcc is included because of helper functions used by gcc.
# For example: __udivdi3 function used for division of 64 bit integers.
# Note: LD_KERNEL LD_FLAGS {*.o files} LD_OPTIONS -o {output elf}. If
#       LD_OPTIONS were places before input files, the linking with -lgcc did 
#       not work.

# Link using the ld program. 
if [ $LINK_USING_LD -eq 1 ]; then
    GCCVER=$($GCC32 -v 2>&1|tail - -n 1|awk '{print $3}')
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
     export LD_KERNEL="$GCC32"
fi

export OBJCOPY="i686-elf-objcopy"
# ---------------------------------------------------------------------------
# Cleanup directories
rm -fr $REPORTSDIR     || exit
rm -fr $TEMPDIR        || exit
rm -fr $DISKTEMPDIR    || exit
rm -fr $OBJDIR         || exit
rm -fr $IMAGEDIR       || exit
rm -fr $LISTDIR        || exit

# Create folders
mkdir $REPORTSDIR      || exit
mkdir $TEMPDIR         || exit
mkdir -p $DISKTEMPDIR  || exit
mkdir $OBJDIR          || exit
mkdir -p $IMAGEDIR     || exit
mkdir $LISTDIR         || exit

# ---------------------------------------------------------------------------
# Build complete kernel and disk image
# ---------------------------------------------------------------------------
[[ $BUILDMODE -ge $KERNEL ]] || exit 0

# Build the bootloaders
bash src/bootloader/x86/build.sh  || exit

# Build kernel
bash src/kernel/build.sh  2>"$REPORTSDIR/build_warnings.txt"

# If build fails, build_warnings.txt will contain errors, as well as warnings,
# otherwise will contain warnings only.
# In case of build failure, the whole file is printed.
if [ $? -ne 0 ]; then
    cat "$REPORTSDIR/build_warnings.txt"
    rm "$REPORTSDIR/build_warnings.txt"
    exit
fi

# ---------------------------------------------------------------------------
# Build the floppy image
echo "    [ Creating disk image ]    "
mkdosfs -C $IMAGEDIR/mos.flp -F 12 1440 || exit

# mount the Disk image
echo "    [ Mounting Disk image ]    "
sudo mount $IMAGEDIR/mos.flp $DISKTEMPDIR || exit
# ---------------------------------------------------------------------------
# Copy the files needed to the floppy
echo "    [ Copy files to floppy ]    "
sudo cp -v $OBJDIR/boot1.flt $DISKTEMPDIR ||exit
sudo cp -v $OBJDIR/kernel.flt $DISKTEMPDIR ||exit

# Unmount the image
echo "    [ Copy of files done. Unmounting image ]    "
sudo umount $DISKTEMPDIR || exit

# Wrtie the bootloader
echo "    [ Writing bootloader to floppy image ]    "
dd conv=notrunc if=$OBJDIR/boot0.flt of=$IMAGEDIR/mos.flp || exit

echo "    [ Report: Storage Utilization ]"
wc -c $OBJDIR/*.flt

# ---------------------------------------------------------------------------
# Lint and compiler warning reports
# ---------------------------------------------------------------------------
[[ $BUILDMODE -ge $LINT ]] || exit 0

echo "    [ Running linting tool ]"
./lint.sh -D__i386__ \
          -D$DEBUG \
          -DDEBUG_LEVEL=$DEBUGLEVEL >"$REPORTSDIR/lint_report.txt" 2>&1 || exit

echo "    [ Generating tags file ]"
ctags -R ./src ./include/ || exit

echo "    [ Cleaning up ]"
rm -f -r $DISKTEMPDIR || exit

# ---------------------------------------------------------------------------
echo "    [ Report: Warning count ]"
WARNCOUNT_GCC=`grep -c -r "warning:" build/reports/build_warnings.txt`
WARNCOUNT_LINT=`grep -c -r "warning:" build/reports/lint_report.txt`
echo "Total compiler warnings: $WARNCOUNT_GCC"
echo "Total lint warnings: $WARNCOUNT_LINT"

# ---------------------------------------------------------------------------
# Build unittest and coverage reports
# ---------------------------------------------------------------------------
[[ $BUILDMODE -ge $ALL ]] || exit 0

echo "    [ Buliding Unittests ]"
bash src/unittests/build.sh || exit

echo "    [ Buliding Coverage report ]"

LCOV_AVAILABLE=`which lcov`
GENHTML_AVAILABLE=`which genhtml`

if [ ! -e "$LCOV_AVAILABLE" ] || [ ! -e "$GENHTML_AVAILABLE" ]; then
    echo "Cannot find lcov and genhtml. Skipping report generation."
else
    ./run.sh unittests > /dev/null 2>&1
    lcov --capture --directory . \
         -rc lcov_branch_coverage=1 \
         --output-file build/coverage/capture.data > /dev/null  || exit
    genhtml build/coverage/capture.data \
            --branch-coverage \
            -o build/coverage/report > /dev/null                || exit
fi

# ---------------------------------------------------------------------------
echo "    [ Done ]"

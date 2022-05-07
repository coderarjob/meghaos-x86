#!/bin/bash

source functions.sh

# ---------------------------------------------------------------------------
FUT_OBJDIR="$OBJDIR/fut"

UNITTEST_BINDIR="build/bin/unittests"

UNITTEST_WOPTS="-Wpedantic   \
                -Wpadded     \
                -Wall        \
                -Wextra      \
                -Wconversion"
MOCK_WOPTS=$UNITTEST_WOPTS
FUT_WOPTS=$UNITTEST_WOPTS

# Note: Requires gcc-multilib package if compiling on a x86_64 machine.
UNITTEST_CC="gcc -std=c99                   \
                 -m32                       \
                 -march=i386                \
                 $UT_WOPTS                  \
                 $GCC_INCPATH               \
                 -D UNITTEST                \
                 -D ARCH=$ARCH              \
                 -D $DEBUG                  \
                 -D DEBUG_LEVEL=$DEBUGLEVEL"
MOCK_CC=$UNITTEST_CC
FUT_CC=$UNITTEST_CC

LD_UNITTEST="gcc -m32"
LD_MOCK=$LD_UNITTEST
LD_FUT=$LD_UNITTEST


# ---------------------------------------------------------------------------
# Clean and create directories
DIRS=(
    "$UNITTEST_BINDIR"
)

for dir in ${DIRS[@]}
do
    rm    -fr "$dir"   || exit
    mkdir -p  "$dir"   || exit
done

# ---------------------------------------------------------------------------
# Test Dependencies
BASE_UNITTEST=('unittests/unittest.c')

C99_CONFORMANCE_TEST_FUT=()
C99_CONFORMANCE_TEST_MOCK=()
C99_CONFORMANCE_TEST_UNITTEST=('unittests/c99_conformance_test.c')

PRINTK_TEST_FUT=('kernel/printk.c')
PRINTK_TEST_MOCK=('mock/kernel/x86/vgadisp.c')
PRINTK_TEST_UNITTEST=('unittests/printk_test.c')

# ---------------------------------------------------------------------------
# Compile mock, FUT and unittest C files.
MOCK_FILES=(
    $C99_CONFORMANCE_TEST_MOCK
    $PRINTK_TEST_MOCK
)

FUT_FILES=(
    $C99_CONFORMANCE_TEST_FUT
    $PRINTK_TEST_FUT

)

UNITTEST_FILES=(
    $BASE_UNITTEST
    $C99_CONFORMANCE_TEST_UNITTEST
    $PRINTK_TEST_UNITTEST
)

compile_cc "$MOCK_CC" "$OBJDIR" "${MOCK_FILES[@]}"
compile_cc "$FUT_CC" "$FUT_OBJDIR" "${FUT_FILES[@]}"
compile_cc "$UNITTEST_CC" "$OBJDIR" "${UNITTEST_FILES[@]}"

# ---------------------------------------------------------------------------
# Link individual tests
link_unittest_add_file "$FUT_OBJDIR" "$C99_CONFORMANCE_TEST_FUT"
link_unittest_add_file "$OBJDIR"     "$C99_CONFORMANCE_TEST_MOCK"
link_unittest_add_file "$OBJDIR"     "$C99_CONFORMANCE_TEST_UNITTEST"
link_unittest_add_file "$OBJDIR"     "$BASE_UNITTEST"
link_unittest "$LD_UNITTEST" "$UNITTEST_BINDIR/c99_conformance_test"

link_unittest_add_file "$FUT_OBJDIR" "$PRINTK_TEST_FUT"
link_unittest_add_file "$OBJDIR"     "$PRINTK_TEST_MOCK"
link_unittest_add_file "$OBJDIR"     "$PRINTK_TEST_UNITTEST"
link_unittest_add_file "$OBJDIR" "$BASE_UNITTEST"
link_unittest "$LD_UNITTEST" "$UNITTEST_BINDIR/printk_test"

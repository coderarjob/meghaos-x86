#!/bin/bash

# Parent script that setups up the global variables and builds both
# architecture dependent and independent unittests. Independent tests are build
# in this script. Architecture specific tests are added and build rules are
# implemented in a separate script, specific for that architecture.

# If a test depends on a platform specific mock file, then that file is added
# using a separate script specific for that architecture.
# ---------------------------------------------------------------------------

source functions.sh
source src/mock/build-${ARCH}.sh
source src/unittests/$ARCH/build.sh

# ---------------------------------------------------------------------------
UNITTEST_BINDIR="build/bin/unittests"
UNITTEST_OBJDIR="build/obj/unittests"

UNITTEST_WOPTS="-Wpedantic   \
                -Wpadded     \
                -Wall        \
                -Wextra      \
                -Wconversion"

# Note: Requires gcc-multilib package if compiling on a x86_64 machine.
UNITTEST_CC="gcc -std=c99                   \
                 -g                         \
                 -m32                       \
                 -march=i386                \
                 --coverage                 \
                 $UT_WOPTS                  \
                 $GCC_INCPATH               \
                 -D UNITTEST                \
                 -D ARCH=$ARCH              \
                 -D $DEBUG                  \
                 -D DEBUG_LEVEL=$DEBUGLEVEL"

LD_UNITTEST="gcc -m32"
LD_UNITTEST_FLAGS="-lgcov"

# ---------------------------------------------------------------------------
# Architecture independent Unit tests
UNITTESTS=(
    "c99_conformance"
    "printk"
)

# Adds tests specific to the current architecture.
add_arch_specific_tests ${UNITTESTS[@]}      # Adds to $UNITTESTS array

# ---------------------------------------------------------------------------
# Clean and create directories

rm    -fr "$UNITTEST_BINDIR"   || exit
rm    -fr "$UNITTEST_OBJDIR"   || exit
mkdir -p  "$UNITTEST_BINDIR"   || exit

# ---------------------------------------------------------------------------
get_arch_independent_test_definition()
{
    local TEST=$1

    case $test in
        c99_conformance)
                        {
                            SRC=('unittests/c99_conformance_test.c'
                                 'unittests/unittest.c')
                        };;
        printk)
                        {
                            SRC=('kernel/printk.c'
                                 'unittests/printk_test.c'
                                 'unittests/unittest.c')
                        };;
        *) return 1;;
    esac
}

# ---------------------------------------------------------------------------
# Builds each unittest. Rules for architecture independent tests are taken from
# 'get_arch_independent_test_defination()' function. Rules for architecture
# dependent tests are taken from 'get_arch_dependent_test_definition()'
# function.

# A test name is matched first with the independent tests, if there is no match
# then, and only then the tests are considered architecture dependent, and
# checked for such a rule.
for test in ${UNITTESTS[@]}
do
    echo -e "\tTEST: $test"

    get_arch_independent_test_definition $test || {
            get_arch_dependent_test_definition $test || {
                    echo "$test" test is not defined.
                    exit
                }
    }

    add_mock_source $test ${SRC[@]}    # Adds to $SRC array.

    TESTOBJDIR="$UNITTEST_OBJDIR/$test"
    compile_cc "$UNITTEST_CC" "$TESTOBJDIR" "${SRC[@]}"
done

# ---------------------------------------------------------------------------
# Link object files for each test

for test in ${UNITTESTS[@]}
do
    TESTOBJDIR="$UNITTEST_OBJDIR/$test"
    OBJ_FILES=`find "$TESTOBJDIR" -name "*.o"`
    $LD_UNITTEST $OBJ_FILES $LD_UNITTEST_FLAGS \
                 -o "$UNITTEST_BINDIR/${test}_test" || exit
done

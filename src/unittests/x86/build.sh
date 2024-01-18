#!/bin/bash

# Helper script called from unittests/build.sh. The specific functions here form
# an interface, which is used by the parent build script to apply build rules
# for architecture dependent tests. Here for x86 architecture.
# ---------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# add_arch_specific_tests
# Appends architecture dependent test names to the list passed in the
# parameter.
# Output is the appended list in a global $UNITTESTS array.
#
# $1   - List of already added unittests. Can also be empty (nothing passed
#        in).
add_arch_specific_tests()
{
    local UNITTESTS_ARRAY=$@
    export UNITTESTS=($UNITTESTS_ARRAY
                      pmm_x86
                      paging_x86
    )
}

# ----------------------------------------------------------------------------
# get_arch_dependent_test_definition
# Returns a list of source files for building an architecture dependent test.
# The files which is added depends on the name of the test.
# Output is the put into a global $SRC array.
# Note: Mock files are added separately by the parent caller script.
#
# $1   - Name of the test.
#
# Example:
# ```
#  printk) {
#              export SRC=( 'kernel/printk.c' 'unittests/printk_test.c'
#                           'unittests/unittest.c')
#          };;
# ```
get_arch_dependent_test_definition()
{
    local TEST=$1

    case $TEST in
        pmm_x86) {
                   export SRC=('kernel/x86/pmm.c'
                               'unittests/common.c'
                               'unittests/x86/pmm_test.c')
                 };;
        paging_x86) {
                   export SRC=('kernel/x86/paging.c'
                               'unittests/common.c'
                               'unittests/x86/paging_test.c')
                 };;
            *) return 1;;
    esac
}

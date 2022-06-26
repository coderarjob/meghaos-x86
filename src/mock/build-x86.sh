#!/bin/bash

# Helper script called from unittests/build.sh. The specific functions here form
# an interface, which is used by the parent build script to add architecture
# specific mock files of a particular test.
# ---------------------------------------------------------------------------

source functions.sh

# ----------------------------------------------------------------------------
# add_mock_source
# Appends mock source files to the list passed in the parameter. The file which
# is added to the list depends on the name of the test.
# Output is the appended list in a global $SRC env variable.
#
# $1   - Name of the test.
# $2   - List of already added source files for building the test. Can also be
#        empty (nothing passed in).
function add_mock_source()
{
    local TEST=$1       ; shift
    local SRC_ARRAY=$@

    case $test in
        printk) {
                    export SRC=( $SRC_ARRAY
                                'mock/kernel/x86/vgadisp.c')
                };;
        pmm)    {
                    export SRC=( $SRC_ARRAY
                                 'mock/kernel/mem.c'
                                 'mock/kernel/x86/boot.c'
                               )
                };;
    esac
}

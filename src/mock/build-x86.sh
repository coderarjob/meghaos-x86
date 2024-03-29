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

    case $TEST in
        printk) {
                    export SRC=( $SRC_ARRAY
                                'mock/kernel/x86/vgadisp.c')
                };;
        pmm)    {
                    export SRC=( $SRC_ARRAY
                                 'mock/kernel/x86/pmm.c'
                                 'mock/common/utils.c'
                               )
                };;
       pmm_x86) {
                    export SRC=( $SRC_ARRAY
                                 'mock/kernel/kstdlib.c'
                                 'mock/common/bitmap.c'
                                 'mock/kernel/pmm.c'
                                 'mock/kernel/x86/boot.c'
                               )
                };;
        bitmap) {
                    export SRC=( $SRC_ARRAY
                                 'mock/common/utils.c'
                               )
                };;
       paging_x86) {
                    export SRC=( $SRC_ARRAY
                                 'mock/kernel/kstdlib.c'
                                 'mock/kernel/pmm.c'
                                 'mock/kernel/x86/paging.c'
                               )
                };;
       kmalloc) {
                    export SRC=( $SRC_ARRAY
                                 'mock/kernel/x86/memmanage.c'
                               )
                };;
       salloc)  {
                    export SRC=( $SRC_ARRAY
                                 'mock/kernel/x86/memmanage.c'
                                 'mock/kernel/kstdlib.c'
                               )
                };;
    esac
}

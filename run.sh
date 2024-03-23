#!/bin/bash

# -----------------------------------------------------------------------------
# run_test_by_name
# Runs the specific test whose name was passed in.
#
# $1 - Unittest file name
#
run_test_by_name()
{
    local TEST=$1

    echo -e -n "==================================\n"
    echo -e -n "Test: $TEST\n"
    echo -e -n "----------------------------------\n"
    "$TEST"
}
# -----------------------------------------------------------------------------

QEMU_REPORT_FILE="./build/reports/qemu-run.txt"
TEST_DIR="./build/bin/unittests"
RUNMODE='QEMU'

[[ $# -gt 0 ]] && [[ "$1" = "unittests" ]] && RUNMODE='TESTS'
[[ $# -gt 1 ]] && [[ "$1" = "unittests" ]] && RUNMODE='ONE_TEST'

case $RUNMODE in
    QEMU)
        {
            OPTS="-m 2561k"
            [[ $# -gt 0 ]] && OPTS="$@"

            # Run emulator and add the output to a reports file
            qemu-system-i386 $OPTS -fda ./build/diskimage/x86/mos.flp \
                                   -boot a                            \
                                   -cpu 486                           \
                                   -debugcon stdio                    \
                                   -no-reboot                         \
                                   -no-shutdown                       \
                                   -d cpu_reset                       \
                                   | tee ${QEMU_REPORT_FILE}

            # Remove the ANSI color codes from the reports file
            sed -i $'s/\033\[[0-9;]*m//g' ${QEMU_REPORT_FILE}
        };;
    TESTS)
        for fn in $TEST_DIR/*; do
            run_test_by_name "$fn"
        done;;

    ONE_TEST)
        run_test_by_name "$TEST_DIR/$2";;

    *)
        echo Invalid run mode.
        echo $0 [unittests [test name]]

        echo Valid test names are:
        ls $TEST_DIR
        exit 1;;
esac

exit 0

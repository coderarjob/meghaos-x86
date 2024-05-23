#!/bin/bash

# -----------------------------------------------------------------------------
# Helper script to run MOS diskimage using QEMU and log its output to a reports
# file. It also runs all or any specific unittest
# -----------------------------------------------------------------------------

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
# fatal
# Prints usage and exits
function fatal()
{
    USAGE="\n$0 operates in these two modes:"
    USAGE+="\nQEMU:\t\t -img=<path to image> -report-out=<report file path> "
    USAGE+="-- <args to qemu>."
    USAGE+="\nUNITTEST:\t -test-dir=<path to test bins > [-name=<test name>].\n"
    USAGE+="Only runs [test name] if provided."

    echo -e $USAGE
    exit 1
}

# -----------------------------------------------------------------------------

QEMU_REPORT_FILE=""
TEST_DIR=""
DISKIMAGE_PATH=""
TEST_NAME=""

RUNMODE="UNKNOWN"

# -----------------------------------------------------------------------------
# Parse arguments and assign appropriate variables
# -----------------------------------------------------------------------------

# $@ though acts as an array, cannot be indexed like ${@[1]}. That is the reason
# for this ARGS array.
declare -a ARGS=("$@")

i=0
skip_next=0

for arg in $@; do
    ((i = $i + 1))
    shift

    [[ $skip_next -eq 1 ]] && skip_next=0 && continue

    case $arg in
        --img       ) {
                        skip_next=1
                        DISKIMAGE_PATH="${ARGS[$i]}"
                        RUNMODE="QEMU"
                      };;
        --report-out) {
                        skip_next=1
                        QEMU_REPORT_FILE="${ARGS[$i]}"
                        RUNMODE="QEMU"
                      };;
        --tests-dir ) {
                        skip_next=1
                        TEST_DIR="${ARGS[$i]}"
                        RUNMODE="TESTS"
                      };;
        --name      ) {
                        skip_next=1
                        TEST_NAME="${ARGS[$i]}"
                        RUNMODE="ONE_TEST"
                      };;
        --          ) break;;
        *           ) echo "Invalid argument $arg" && fatal
    esac
done

# -----------------------------------------------------------------------------
# Validate inputs provided
# -----------------------------------------------------------------------------
[[ $RUNMODE == "QEMU" ]] && [[ -z $QEMU_REPORT_FILE ]] && \
    echo "Reports file not provided. Invalid QEMU mode." && fatal
[[ $RUNMODE == "QEMU" ]] && [[ -z $DISKIMAGE_PATH ]] && \
    echo "Disk image path not provided. Invalid QEMU mode." && fatal

[[ $RUNMODE == "TESTS" ]] && [[ -z $TEST_DIR ]] && \
    echo "Tests binary path not provided. Invalid TESTS mode." && fatal

[[ $RUNMODE == "ONE_TEST" ]] && [[ -z $TEST_DIR ]] && \
    echo "Tests binary path not provided. Invalid ONE_TEST mode." && fatal
[[ $RUNMODE == "ONE_TEST" ]] && [[ -z $TEST_NAME ]] && \
    echo "Test name not provided. Invalid ONE_TEST mode." && fatal
# -----------------------------------------------------------------------------
case $RUNMODE in
    QEMU)
        {
            OPTS="-m 2561k"
            [[ $# -gt 0 ]] && OPTS="$@"

            # Run emulator and add the output to a reports file
            qemu-system-i386 $OPTS -fda ${DISKIMAGE_PATH}             \
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
        run_test_by_name "$TEST_DIR/$TEST_NAME";;

    *)
        echo Invalid run mode.
        exit 1;;
esac

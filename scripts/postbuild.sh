#!/bin/bash

# -----------------------------------------------------------------------------
# Helper script to run various post-build tools for example ctags, cscope. It
# can also generate unittest coverage reports.
# -----------------------------------------------------------------------------

# Valid modes: GENTAG, GENCOV
MODE=GENTAG

# -----------------------------------------------------------------------------
# Prints Usage message and exits
function fatal()
{
    USAGE="\nGenerates either ctags/cscope db files or unittests coverage.\n"
    USAGE+="Usage:\n"
    USAGE+="\t$0 [--test-dir=<path to tests bins> "
    USAGE+="--cov-dir=<path to reports dir>]\n\n"
    USAGE+="If no arguments are given, it runs in GENTAG mode and generates"
    USAGE+="ctags and cscope db files, otherwise uses the provided input to"
    USAGE+="run the unittests and generate coverage report."

    echo -e $USAGE
    exit 1
}
# -----------------------------------------------------------------------------
# Parse Input arguments
# -----------------------------------------------------------------------------
for arg in "$@"; do
    case $arg in
        --tests-dir=*) {
                        TEST_DIR=${arg##*=}
                        MODE=GENCOV
                      };;
        --cov-dir=* ) {
                        COV_DIR=${arg##*=}
                        MODE=GENCOV
                      };;
        *           ) echo "Invalid arg $arg" && fatal
    esac
done

# -----------------------------------------------------------------------------
# Validate input
# -----------------------------------------------------------------------------
[[ $MODE == "GENCOV" ]] && [[ -z $TEST_DIR ]] && \
    echo "Test directory path not provided. Cannot continue." && fatal
[[ $MODE == "GENCOV" ]] && [[ -z $COV_DIR ]] && \
    echo "Coverage directory path not provided. Cannot continue." && fatal

# -----------------------------------------------------------------------------

function gentag()
{
    command -v ctags > /dev/null || {
        echo "ctags not found. Skipping"
        exit
    }

    command -v cscope > /dev/null || {
        echo "scope not found. Skipping"
        exit
    }

    ctags -R ./src ./include
    cscope -R -k -b -I ./include
}

function gencov()
{
    local CovDataFile="${COV_DIR}/capture.data"
    local CovReportDirPath="${COV_DIR}/report"

    ./scripts/run.sh --tests-dir "${TEST_DIR}" > /dev/null 2>&1 || exit

    lcov --capture --directory .    \
         -rc lcov_branch_coverage=1 \
         --output-file $CovDataFile > /dev/null  || exit

    genhtml $CovDataFile      \
            --branch-coverage \
            --dark-mode \
            -o $CovReportDirPath > /dev/null     || exit
}
# -----------------------------------------------------------------------------

case $MODE in
    "GENTAG") gentag;;
    "GENCOV") gencov;;
    *       ) echo "Invalid mode $MODE" && exit 1
esac

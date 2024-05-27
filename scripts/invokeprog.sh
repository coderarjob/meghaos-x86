#!/bin/bash

#-------------------------------------------------------------------------------
# To be used to invoke a script/program from build system at build time. There
# is no way to pass arguments from the build system (ex make, ninja etc) to the
# program invoked by it. This is why the script is requried. The arguments are
# passed in ARGS environment variable which this script passed to the invoked
# program in an transparent way.
# Example:
# make ARGS="-S -s" run
#-------------------------------------------------------------------------------

[[ $# -lt 1 ]] && \
    echo -e "Usage: $0 <program/script path>\n" \
            '\tArguments to <program> are read from `ARGS` env variable' \
            && exit 1

PROG="$1"; shift
${PROG} "$@" ${ARGS}

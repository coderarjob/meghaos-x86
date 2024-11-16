#! /bin/sh

LINTNAME='cppcheck'
LINTPATH=`which $LINTNAME`

if [ ! -e "$LINTPATH" ]; then
    echo Cannot find linting software. Install $LINTNAME.
    exit 1
fi

OUTFILE=$1

cppcheck --output-file="$OUTFILE" \
         --language=c \
         --enable=warning,performance,portability,style \
         --template=gcc \
         --std=c99 \
         --inconclusive \
         --quiet \
         --platform=unix32 \
         -I include \
         -D__i386__ \
         -DDEBUG \
         -DPORT_E9_ENABLED \
         src/common src/kernel/ || exit

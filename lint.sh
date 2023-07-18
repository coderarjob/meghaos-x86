#! /bin/sh

LINTNAME='cppcheck'
LINTPATH=`which $LINTNAME`

if [ ! -e "$LINTPATH" ]; then
    echo Cannot find linting software. Install $LINTNAME.
    exit 1
fi

EXTRAARGS=$@

cppcheck --language=c \
         --enable=warning,performance,portability,style \
         --template=gcc \
         --std=c99 \
         --inconclusive \
         --quiet \
         --platform=unix32 \
         -I include \
         $EXTRAARGS \
         src/common src/kernel/ || exit

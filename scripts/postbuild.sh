#!/bin/bash

# -----------------------------------------------------------------------------
# Helper script to run various post-build tools for example ctags, cscope. It
# can also generate unittest coverage reports.
# -----------------------------------------------------------------------------

command -v ctags > /dev/null || {
    echo "ctags not found. Skipping" 
    exit 1
}

command -v cscope > /dev/null || {
    echo "scope not found. Skipping" 
    exit 1
}

ctags -R ./src ./include
cscope -R -k -b -I ./include

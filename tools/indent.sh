#!/bin/bash

pushd ~/meghaos-v2/

# --------------------------------------------------------------------------
# Space before every (
# Note however this rule, does not apply to macro definations. 
# There should be no space between macro name and '('. This will be fixed 
# later in the 'fix'.
# ---------------------------------------------------------------------------
E="\(\w\)\((\)/\1\ ("
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/$E/g" {} \;

     # ---- Appy Fix for Macros
     # There should be no space between macro name and '('. 
     # '#define macro (..)' fixed to 'define macro(..)'

    E="\(\W*#\W*define\W\+\w\+\)\ (/\1("
    find -type f \( -name "*.c" -o -name "*.h" \) \
         -exec sed -i "s/$E/g" {} \;
    # ----

# --------------------------------------------------------------------------
# Space around binary and ternary operators. 
# `=  +  -  <  > * /  %  |  &  ^  <=  >=  ==  !=  ?  :`
# ---------------------------------------------------------------------------

# Operators `=  +  -  <  >  %  |  &  ^  <=  >=  ==  !=  ?  :`
AR=("= + - < > % | & ^ <= >= == != ? :")
for op in ${AR[@]}; do
    E="\(\w\)\($op\)\(\w\)/\1\ \2\ \3"
    find -type f \( -name "*.c" -o -name "*.h" \) \
         -exec sed -i "s/${E}/g" {} \;
done

# Operators `*`
E="\(\w\)\(*\)\(\w\)/\1\ \2\ \3"
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/$E/g" {} \;

    # ---- Appy Fix for (%x, %llx -> % x, % llx)
    E="\(%\ \)\(x\|llx\)/%\2"
    find -type f \( -name "*.c" -o -name "*.h" \) \
         -exec sed -i "s/$E/g" {} \;
    # ----
# --------------------------------------------------------------------------
popd

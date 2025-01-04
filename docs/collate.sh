#!/bin/bash

################################################################################
replace_text_in_file()
{
    if [ $# -ne 4 ]; then
        echo "$0 - Invalid number of arguments"
        exit 1
    fi

    local template_file=$1
    local outfile=$2
    local pattern=$3
    local replacement=$4

    # Sed will not work in this case because it cannot handle newlines in the
    # replacement text.
    #
    # AWK Manpage: An & in the replacement text is replaced with the text that
    # was actually matched. Use \& to get a literal &.
    replacement=${replacement//'&'/'\\&'} 
    awk -v MD="$replacement" '{sub(/'$pattern'/, MD); print;}' \
        $template_file > $outfile
}

form_pattern()
{
    local categories=$@
    export RETURN="%{$categories}%"
}

################################################################################

TEMPLATE_FILE=index_template.md
INDEX_FILE=index.md
DIR=`dirname "$0"`

doc_categories=('guideline independent'
                'guideline x86'
                'guideline independent obsolete'
                'guideline x86 obsolete'
                'feature independent'
                'feature x86'
                'feature independent obsolete'
                'feature x86 obsolete'
                'note independent'
                'note x86'
                'note independent obsolete'
                'note independent feature'
                'note x86 obsolete')

################################################################################
pushd "$DIR"

temp_template_file="./.$TEMPLATE_FILE"
cp "$TEMPLATE_FILE" "$temp_template_file"

IFS='' # Loop should not split by space, which is the default value of IFS.
for cate in ${doc_categories[@]}; do
    list=$(./titles.sh $cate)
    form_pattern $cate
    pattern=$RETURN
    replace_text_in_file "$temp_template_file" "$INDEX_FILE" "$pattern" "$list"
    cp "$INDEX_FILE" "$temp_template_file"
done

rm -f "$temp_template_file"

popd

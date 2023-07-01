#!/bin/sh

TAG_LINE_PATTERN="categories"
TAG_PAT=""

if [ $# -eq 0 ]; then
    echo "Usage: $0 category [category] ..."
    exit 1
fi

for arg in $@; do
    TAG_PAT="${TAG_PAT}.*$arg"
done
TAG_PAT="${TAG_PAT}[\ \t]*$"

FILES_WITH_TITLE=$(grep -a -B1 --no-group-separator   \
                   -ire "^$TAG_LINE_PATTERN$TAG_PAT" | \
                   grep -v "$TAG_LINE_PATTERN")

IFS=$'\n' # $'<ch>' expands <ch> to the escape character. Here its new line.
for data in ${FILES_WITH_TITLE[@]}; do
    filename=${data%.md-*}  # Get filename. Eveything left of the first `.md`
    title=${data#*-##\ }      # Title style 2. Everything right of the first -#
    title=${title#\ *}      # Remove leading space
    title=${title//\ /-}    # Replace spaces with `-` (To be valid links)
    title=${title//\?/}     # Remove `?`s (To be valid links)
    echo "- [$title]($filename.md#$title)"
done

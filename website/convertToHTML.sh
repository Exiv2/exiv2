#!/bin/bash

##
# convertToHTML.sh
# generate convert.html by reading src/convert.cpp
##

declare -a from=($(grep mdExif, ../src/convert.cpp | cut '-d"' -f 2))
declare -a to=($(grep mdExif,   ../src/convert.cpp | cut '-d"' -f 4))
count=${#from[@]}

index=0
(
    echo '<html><body style="background:pink;margin-left:10px">'
    echo '<h1>Exiv2 Convert Table</h1>'
    echo '<p>Generated at: ' $(date) '</p>'

    echo   '<table>'
    echo   '<tr><th style="text-align:left;font-style:italic">From</th><th>To</th></tr>'
    while [ $index -lt $count ]; do
        printf '<tr><td>%s</td><td>%s</td></tr>\n' "${from[index]}" "${to[index]}"
        index=$((index+1))
    done
    echo '</table>'

    echo '<html><body><table>'
) > convert.html

open convert.html

# That's all Folks
##

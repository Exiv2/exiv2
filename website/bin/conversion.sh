#!/bin/bash

##
# conversion.sh
# generate conversion.html by parsing src/convert.cpp
# Thanks to Robin Mills for initial code
##

declare -a from=($(grep "mdExif,\|mdIptc," ../src/convert.cpp | cut '-d"' -f 2))
declare -a to=($(grep "mdExif,\|mdIptc,"   ../src/convert.cpp | cut '-d"' -f 4))
count=${#from[@]}

index=0
(
    echo   '<table class="table table-striped">'
    echo   '<tr><th>Between Exif tags or Iptc datasets</th><th>and XMP properties</th></tr>'
    while [ $index -lt $count ]; do
        printf '<tr><td>%s</td><td>%s</td></tr>\n' "${from[index]}" "${to[index]}"
        index=$((index+1))
    done
    echo '</table>'

) > ../website/var/__conversion__

# That's all Folks
##

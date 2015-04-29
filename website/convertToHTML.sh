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
    echo '<html><head><title>Exiv2 Convert Table</title></head>'
    echo '<style>'
    cat   << STYLE
	body {
		font                : 11px verdana, arial, helvetica, sans-serif;
		line-height         : 12px;
		margin-left         : 20px;
		margin-top          : 20px;
		margin-bottom       : 20px;
		color               : blue;
		background-color    : #44bbff;
		width               : 850px;
	}
	th {
		font                : 14px verdana, arial, helvetica, sans-serif;
		font-weight         : bold;
		font-style          : italic;
		line-height         : 12px;
		color               : black;
		background-color    : white;
		text-align          : left;
	}
STYLE
    echo '</style>'

    echo '<body><img src="Exiv2Logo.png">'
    echo '<h1>Exiv2 Convert Table</h1>'
    echo '<p>Generated at: ' $(date) '</p>'

    echo   '<table>'
    echo   '<tr><th>From</th><th>To</th></tr>'
    while [ $index -lt $count ]; do
        printf '<tr><td>%s</td><td>%s</td></tr>\n' "${from[index]}" "${to[index]}"
        index=$((index+1))
    done
    echo '</table>'

    echo '</body></html>'
) > convert.html

open convert.html

# That's all Folks
##

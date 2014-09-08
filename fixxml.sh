#!/bin/bash

for i in $(find . -name "*proj" | grep msvc); do
    
    ib="$i.bak"
    mv "$i" "$ib"
    xmllint --format --pretty 0 "$ib" | tr -d $'\n' > "$i"
    echo $(wc -l "$ib" "$i")
    rm -rf "$ib"
done

# That's all Folks!
##
 
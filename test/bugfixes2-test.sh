#!/bin/bash
# regression tests (for GitHub bugfixes post #200)

source ./functions.source
errors='0'

(   cd "$testdir/.."

    for t in 223 226 227; do
    	echo -n $t ''
    	out=$(./bugfix-$t.sh 2>&1)
    	if [ $? != 0 ]; then
    		echo '' >3
    		echo ''
    		echo $out
    		errors=$(($errors +  1))
    	fi
    done
    echo '' >3
    echo ''
	echo "errors = $errors"
    if [ $errors != '0' ]; then
    	exit 199
    fi
) 3>&1

# That's all Folks!
##

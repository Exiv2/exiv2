#!/usr/bin/env bash
# Test driver for image file i/o

source ./functions.source

(   cd "$testdir"

    errors=0
    test_files="table.jpg smiley2.jpg ext.dat"
    echo
    printf "Io tests"
    for i in $test_files; do ioTest $i; done

    printf "\n---------------------------------------------------------\n"
    if [ $errors -eq 0 ]; then
       echo 'All test cases passed'
    else
       echo $errors 'test case(s) failed!'
    fi
)

# That's all Folks!
##
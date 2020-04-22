#!/usr/bin/env bash
# Test driver for image file i/o

source ./functions.source

(   cd "$testdir"

    errors=0
    test_files="table.jpg smiley2.jpg ext.dat"
    echo
    printf "file io tests"
    for i in $test_files; do ioTest $i; done

    printf "\n---------------------------------------------------------\n"
    if [ $errors -eq 0 ]; then
       echo 'All test cases passed'
    else
       echo $errors 'test case(s) failed!'
    fi
)

# Test http I/O
(   cd "${testdir}/.."                                # testdir is the tmp output directory
    >&2 printf "*** HTTP tests begin\n"
	port=9999
	url=http://0.0.0.0:$port
	python3 -m http.server $port 2>&1 > /dev/null &   # start a background local HTTP server in the "real" test directory
    sleep 2                                           # wait for it to init
	cd "$testdir"
    test_files="table.jpg Reagan.tiff exiv2-bug922a.jpg"
    for i in $test_files; do
    	runTest exiv2 -pa -g City -g DateTime $url/data/$i
    done
    >&2 printf "*** HTTP tests end\n"
    kill $!                                           # kill the server

)  | tr -d '\r' | sed 's/[ \t]+$//' > $results
reportTest


# That's all Folks!
##
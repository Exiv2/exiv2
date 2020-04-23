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

# these function will be added to functions_source
startHttpServer() {
    cd "${testdir}/.."                   # testdir is the tmp output directory
    if [ ! -z $EXIV2_PORT ]; then port=$EXIV2_PORT ; else port=1276; fi
    url=http://0.0.0.0:$port
    jobs=$(jobs | wc -l)
    python3 -m http.server $port &       # start a background local HTTP server in the "real" test directory
    sleep 2                              # wait for it to init or die!
    
    if [ $(jobs | wc -l) != $jobs ]; then
        exiv2_httpServer=$!
    else
        >&2 printf "*** startHttpServer failed to start on port $port ***\n"
    fi
}
closeHttpServer() {
    if [ ! -z $exiv2_httpServer ]; then
        kill  $exiv2_httpServer                       # kill the server
    fi 
}

# Test http I/O
startHttpServer
if [ ! -z $exiv2_httpServer ]; then
    (   cd "${testdir}" 
        >&2 printf "*** HTTP tests begin\n"

        cd "$testdir"
        test_files="table.jpg Reagan.tiff exiv2-bug922a.jpg"
        for i in $test_files; do
            runTest exiv2 -pa -g City -g DateTime $url/data/$i
        done
        >&2 printf "*** HTTP tests end\n"
    )  | tr -d '\r' | sed 's/[ \t]+$//' > $results
    reportTest
fi
closeHttpServer

# That's all Folks!
##
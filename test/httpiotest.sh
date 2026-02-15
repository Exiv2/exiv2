#!/usr/bin/env bash
# Test driver for httptest and httpIo

source ./functions.source

##
#function to test httpio class in basicio.cpp
httpIoTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/remoteio.txt
    dot=.

    # print out the metadata
    runTest remotetest $1 "--nocurl" > $test

    #check results
    diffCheckAscii $test $good
    printf $dot
}

(   cd "$testdir"

    # check environmental variable
    if [ -z $EXIV2_AWSUBUNTU_HOST ]; then
        echo "***" please set the environmental variable EXIV2_AWSUBUNTU_HOST "***"
        exit 1
    fi

    errors=0
    testfile="conntest.jpg"
    iopngfiles+=(remoteio{1..5}.png)
    iojpgfiles+=(remoteio{6..10}.jpg)

    # test connection (basic sanity test)
    printf 'http test connection '
    TEST_CON=$("$bin"/conntest http://$EXIV2_AWSUBUNTU_HOST/$testfile)
    if [[ "$TEST_CON" == OK* ]]; then
        #Tests for httpIo
        printf 'OK\nHTTP IO '
        for name in ${iopngfiles[@]}; do
            httpIoTest "http://$EXIV2_AWSUBUNTU_HOST/$name"
        done
        for name in ${iojpgfiles[@]}; do
            httpIoTest "http://$EXIV2_AWSUBUNTU_HOST/$name"
        done
        if [ $errors -eq 0 ]; then
            printf '\nAll test cases passed\n'
        else
            printf "\n---------------------------------------------------------\n"
            echo $errors 'httpIo failed!'
        fi
    else
        echo $TEST_CON
        printf "FAIL\nnot run httpIo\n"
    fi
)

# That's all Folks!
##
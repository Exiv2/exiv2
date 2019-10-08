#!/usr/bin/env bash
# Test driver for CurlIO

source ./functions.source


##
#function to test remote read access in basicio.cpp
RemoteReadTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/${filename}.txt
    dot=.
    # run tests
    runTest exifprint $1 "--curl" > $test
    #check results
    diffCheckAscii $test $good

    printf $dot
}

##
#function to test remoteio read/write access in basicio.cpp
RemoteIOTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/remoteio.txt
    dot=.

    # print out the metadata
    runTest remotetest $1 "--curl" > $test

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
    if [ -z $EXIV2_AWSUBUNTU_USERNAME ]; then
        echo "***" please set the environmental variable EXIV2_AWSUBUNTU_USERNAME "***"
        exit 1
    fi
    if [ -z $EXIV2_AWSUBUNTU_PASSWORD ]; then
        echo "***" please set the environmental variable EXIV2_AWSUBUNTU_PASSWORD "***"
        exit 1
    fi

    errors=0
    tesfile="conntest.jpg"
    files+=(remoteImg{0..9}.jpg)
    iopngfiles+=(remoteio{1..5}.png)
    iojpgfiles+=(remoteio{6..10}.jpg)
    USE_CURL=$("$bin"/exiv2 -v -V | grep ^curlprotocols= | sed s/curlprotocols=//)
    if [ "$USE_CURL" == "" ]; then
        #Skip remoteIo test cases
        echo 'Curl is not used. Skip remoteio test cases.'
    else
        # HTTP protocol
        if [[ "$USE_CURL" == *http* ]]; then
            # test connection
            printf 'http test connection '
            TEST_CON=$("$bin"/conntest http://$EXIV2_AWSUBUNTU_HOST/$testfile)
            if [[ "$TEST_CON" == OK* ]]; then
                errors=0
                printf 'OK\nHTTP  IO '
                for name in ${iopngfiles[@]}; do
                    RemoteIOTest "http://$EXIV2_AWSUBUNTU_HOST/$name"
                done
                for name in ${iojpgfiles[@]}; do
                    RemoteIOTest "http://$EXIV2_AWSUBUNTU_HOST/$name"
                done
                if [ $errors -eq 0 ]; then
                    printf '\nAll test cases passed\n'
                else
                    echo $errors ' test cases failed!'
                fi
            else
                echo $TEST_CON
                printf "FAIL\nnot run httpIo\n"
            fi
        else
            echo 'Curl doesnt support HTTP'
        fi

        # HTTPS protocol
        if [[ "$USE_CURL" == *https* ]]; then
            # test connection
            printf 'https test connection '
            TEST_CON=$("$bin"/conntest https://$EXIV2_AWSUBUNTU_HOST/$testfile)
            if [[ "$TEST_CON" == OK* ]]; then
                errors=0
                printf 'OK\nHTTPS IO '
                for name in ${iopngfiles[@]}; do
                    RemoteIOTest "https://$EXIV2_AWSUBUNTU_HOST/$name"
                done
                for name in ${iojpgfiles[@]}; do
                    RemoteIOTest "https://$EXIV2_AWSUBUNTU_HOST/$name"
                done
                if [ $errors -eq 0 ]; then
                    printf '\nAll test cases passed\n'
                else
                    echo $errors ' test cases failed!'
                fi
            else
                echo $TEST_CON
                printf "FAIL\nnot run httpsIo\n"
            fi
        else
            echo 'Curl doesnt support HTTPS'
        fi

        # FTP protocol
        if [[ "$USE_CURL" == *ftp* ]]; then
            # test connection
            printf 'ftp test connection '
            TEST_CON=$("$bin"/conntest ftp://"$EXIV2_AWSUBUNTU_USERNAME"_ftp:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/$testfile)
            if [[ "$TEST_CON" == OK* ]]; then
                errors=0
                printf 'OK\nFTP READ '
                for name in ${files[@]}; do
                    RemoteReadTest "ftp://"$EXIV2_AWSUBUNTU_USERNAME"_ftp:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/$name"
                done
                if [ $errors -eq 0 ]; then
                    printf '\nAll test cases passed\n'
                else
                    echo $errors ' test cases failed!'
                fi
            else
                echo $TEST_CON
                printf "FAIL\nnot run FTP read\n"
            fi
        else
            echo 'Curl doesnt support FTP'
        fi
    fi
)

# That's all Folks!
##
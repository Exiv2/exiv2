#!/usr/bin/env bash
# Test driver for SshIo

source ./functions.source

##
#function to test remote read access in basicio.cpp
SFTPReadTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/${filename}.txt
    dot=.
    # run tests
    runTest exifprint $1 > $test
    #check results
    diffCheckAscii $test $good

    printf $dot
}

##
#function to test remote read access in basicio.cpp
SshIOTest()
{
    arg=$1
    scheme=${arg:0:4}

    src=$(basename "$arg")
    filename=${src%.*}
    test=${filename}.txt
    good=$datapath/remoteio.txt
    dot=.

    # print out the metadata
    runTest remotetest $1 > $test

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

    testfile="conntest.jpg"
    files+=(remoteImg{0..9}.jpg)
    iopngfiles+=(remoteio{1..5}.png)
    iojpgfiles+=(remoteio{6..10}.jpg)
    USE_SSH=$("$bin"/exiv2 -v -V | grep ^ssh= | sed s/ssh=//)
    if [ "$USE_SSH" == "0" ]; then
        #Skip remoteIo test cases
        echo 'Ssh is not used. Skip sshio test cases.'
    else
        # test connection
        printf 'sftp test connection '
        TEST_CON=$("$bin"/conntest sftp://"$EXIV2_AWSUBUNTU_USERNAME"_sftp:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/var/www/$testfile)
        if [[ "$TEST_CON" == OK* ]]; then
            # SFTP protocol
            errors=0
            printf 'OK\nSFTP READ '
            for name in ${files[@]}; do
                SFTPReadTest "sftp://"$EXIV2_AWSUBUNTU_USERNAME"_sftp:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/var/www/$name"
            done
            if [ $errors -eq 0 ]; then
                printf '\nAll test cases passed\n'
            else
                echo $errors ' test cases failed!'
            fi
        else
            echo $TEST_CON
            printf "FAIL\nnot run SFTP read\n"
        fi

        # test connection
        printf 'ssh test connection '
        TEST_CON=$("$bin"/conntest ssh://$EXIV2_AWSUBUNTU_USERNAME:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/sshtest/$testfile)
        if [[ "$TEST_CON" == OK* ]]; then
            # SSH protocol
            errors=0
            printf 'OK\nSSH IO '
            for name in ${iopngfiles[@]}; do
                SshIOTest "ssh://$EXIV2_AWSUBUNTU_USERNAME:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/sshtest/$name"
            done
            for name in ${iojpgfiles[@]}; do
                SshIOTest "ssh://$EXIV2_AWSUBUNTU_USERNAME:$EXIV2_AWSUBUNTU_PASSWORD@$EXIV2_AWSUBUNTU_HOST/sshtest/$name"
            done
            if [ $errors -eq 0 ]; then
                printf '\nAll test cases passed\n'
            else
                echo $errors ' test cases failed!'
            fi
        else
            echo $TEST_CON
            printf "FAIL\nnot run SSH read\n"
        fi
    fi
)
# That's all Folks!
##
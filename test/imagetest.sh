#!/usr/bin/env bash
# Test driver for image file i/o

# **********************************************************************
# main
source ./functions.source

(   cd "$testdir"

    test_files="table.jpg smiley1.jpg smiley2.jpg"

    erase_test_files="glider.exv \
                      iptc-noAPP13.jpg \
                      iptc-psAPP13-noIPTC.jpg \
                      iptc-psAPP13-noIPTC-psAPP13-wIPTC.jpg \
                      iptc-psAPP13s-noIPTC-psAPP13s-wIPTC.jpg \
                      iptc-psAPP13s-wIPTC-psAPP13s-noIPTC.jpg \
                      iptc-psAPP13s-wIPTCs-psAPP13s-wIPTCs.jpg \
                      iptc-psAPP13-wIPTC1-psAPP13-wIPTC2.jpg \
                      iptc-psAPP13-wIPTCbeg.jpg \
                      iptc-psAPP13-wIPTCempty.jpg \
                      iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg \
                      iptc-psAPP13-wIPTCend.jpg \
                      iptc-psAPP13-wIPTCmid1-wIPTCempty-wIPTCmid2.jpg \
                      iptc-psAPP13-wIPTCmid.jpg \
                      iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg"

    errors=0

    echo
    printf "Erase all tests"
    for i in $test_files $erase_test_files; do eraseTest $i; done

    echo
    printf "Copy all tests"
    c=0
    for src in $test_files; do
        c=`expr $c + 1`
        for dst in $test_files; do copyTest $c $src $dst; done
    done

    echo
    printf "Copy iptc tests"
    c=0
    for src in $test_files; do
        c=`expr $c + 1`
        for dst in $test_files; do iptcTest $c $src $dst; done
    done

    printf "\n---------------------------------------------------------\n"
    if [ $errors -eq 0 ]; then
        echo 'All test cases passed'
    else
        echo $errors 'test case(s) failed!'
    fi
)

# That's all Folks!
##

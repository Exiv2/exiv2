#!/usr/bin/env bash
# Test driver for Iptc metadata

source ./functions.source

(   cd "$testdir"

    errors=0

    test_files="glider.exv \
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
                iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg \
                smiley1.jpg \
                smiley2.jpg \
                table.jpg"

    echo
    printf "Read tests"
    for i in $test_files; do printTest $i; done

    echo
    printf "Remove tests"
    for i in $test_files; do removeTest $i; done

    echo
    printf "Add/Mod tests"
    for i in $test_files; do addModTest $i; done

    echo
    printf "Extended tests"
    for i in $test_files; do extendedTest $i; done

    printf "\n---------------------------------------------------------\n"
    if [ $errors -eq 0 ]; then
       echo 'All test cases passed'
    else
       echo $errors 'test case(s) failed!'
    fi
)

# That's all Folks!
##
#!/bin/bash
# Test driver for image file i/o

ioTest()
{
    src=$datapath/$1
    out1=${1}.1
    out2=${1}.2

    #run tests
    runTest iotest $src $out1 $out2
    if [ $? -ne 0 ]; then
       errors=`expr $errors + 1`
       return
    fi 

    #check results
    diffCheck $out1 $src 
    diffCheck $out2 $src 
    printf "."
}

# Make sure to pass the test file first and the known good file second
diffCheck() 
{
    test=$1
    good=$2
    
    #run diff and check results
    diff -q --binary $test $good
    if [ $? -ne 0 ]; then
       errors=`expr $errors + 1`
    else
       rm $test
    fi 
}

# **********************************************************************
# main
source ./functions.source

datapath="../data"

test_files="table.jpg smiley2.jpg ext.dat"

errors=0
cd ./tmp
echo

printf "Io tests"
for i in $test_files; do ioTest $i; done

printf "\n---------------------------------------------------------\n"
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi

# That's all Folks!
##
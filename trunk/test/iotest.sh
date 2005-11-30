#! /bin/sh
# Test driver for image file i/o

ioTest()
{
    src=$datapath/$1
    out1=${1}.1
    out2=${1}.2

    #run tests
    $binpath/iotest $src $out1 $out2
    if [ $? -ne 0 ]; then
       errors=`expr $errors + 1`
       return
    fi 

    #check results
    diffCheck $out1 $src 
    diffCheck $out2 $src 
    echo -n "."
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

LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
binpath="$VALGRIND ../../src"
datapath="../data"

test_files="table.jpg smiley2.jpg ext.dat"

errors=0
cd ./tmp
echo

echo -n "Io tests"
for i in $test_files; do ioTest $i; done

echo -e "\n---------------------------------------------------------"
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi

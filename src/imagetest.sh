#! /bin/bash
# Test driver for image file i/o

eraseTest()
{
    src=$1
    test=${src}.etst
    good=${src}.egd

    #setup
    cp $src $test

    #run tests
    ../src/metacopy $test $test

    #check results
    diffCheck $test $good
    echo -n "."
}

copyTest()
{
    num=$1
    src=$2
    dst=$3
    test=${dst}.c${num}tst
    good=${dst}.c${num}gd

    #setup
    cp $dst $test

    #run tests
    ../src/metacopy -a $src $test

    #check results
    diffCheck $test $good
    echo -n "."
}

iptcTest()
{
    num=$1
    src=$2
    dst=$3
    test=${dst}.i${num}tst
    good=${dst}.i${num}gd

    #setup
    cp $dst $test

    #run tests
    ../src/metacopy -ip $src $test

    #check results
    diffCheck $test $good
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
       let ++errors
    else
       rm $test
    fi 
}

test_files="table.jpg smiley1.jpg smiley2.jpg"

let errors=0
cd ../test
echo

echo -n "Erase all tests"
for i in $test_files; do eraseTest $i; done
eraseTest "glider.exv" #extra test

echo -ne "\nCopy all tests"
let c=0
for src in $test_files; do
    let ++c
    for dst in $test_files; do copyTest $c $src $dst; done
done

echo -ne "\nCopy iptc tests"
let c=0
for src in $test_files; do
    let ++c
    for dst in $test_files; do iptcTest $c $src $dst; done
done

echo -e "\n---------------------------------------------------------"
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi

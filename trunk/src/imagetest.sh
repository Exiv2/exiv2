#! /bin/sh
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
}


# Make sure to pass the test file first and the know good file second
diffCheck() {

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

test_files=("table.jpg" "smiley1.jpg" "smiley2.jpg")

let errors=0
cd ../test
echo

echo "Erase all tests..."
foreach i ($test_files); eraseTest $i; end
eraseTest "glider.exv" #extra test

echo "Copy all tests..."
let c=0
foreach src ($test_files)
    let ++c
    foreach dst ($test_files); copyTest $c $src $dst; end
end

echo "Copy iptc tests..."
let c=0
foreach src ($test_files)
    let ++c
    foreach dst ($test_files); iptcTest $c $src $dst; end
end

echo '---------------------------------------------------------'
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi

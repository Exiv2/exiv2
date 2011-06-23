#! /bin/sh
# Test driver for image file i/o

eraseTest()
{
    src=$1
    test=${src}.etst
    good=$datapath/${src}.egd

    #setup
    cp $datapath/$src $test

    #run tests
    $bin/metacopy $test $test

    #check results
    diffCheck $test $good
    printf "."
}

copyTest()
{
    num=$1
    src=$2
    dst=$3
    test=${dst}.c${num}tst
    good=$datapath/${dst}.c${num}gd

    #setup
    cp $datapath/$dst $test

    #run tests
    $bin/metacopy -a $datapath/$src $test

    #check results
    diffCheck $test $good
    printf "."
}

iptcTest()
{
    num=$1
    src=$2
    dst=$3
    test=${dst}.i${num}tst
    good=$datapath/${dst}.i${num}gd

    #setup
    cp $datapath/$dst $test

    #run tests
    $bin/metacopy -ip $datapath/$src $test

    #check results
    diffCheck $test $good
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

if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi

datapath="../data"

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
cd ./tmp
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

#! /bin/sh
# Test driver for Iptc metadata

printTest()
{
    src=$1
    test=${src}.iptst
    good=$datapath/${src}.ipgd

    #run tests
    $binpath/iptcprint $datapath/$src > $test

    #check results
    diffCheck $test $good
    echo -n "."
}

removeTest()
{
    src=$1
    tmp="temp"
    test=${src}.irtst
    good=$datapath/${src}.irgd

    #setup
    cp $datapath/$src $tmp

    #run tests
    $binpath/iptctest $tmp <<-eoc
		r Iptc.Application2.Byline
		r Iptc.Application2.Caption
		r Iptc.Application2.Keywords
		r Iptc.Application2.Keywords
		r Iptc.Application2.Keywords
		r Iptc.Application2.CountryName
eoc
    $binpath/iptcprint $tmp > $test

    #check results
    diffCheck $test $good
    echo -n "."
    rm $tmp
}

addModTest()
{
    src=$1
    tmp="temp"
    test=${src}.iatst
    good=$datapath/${src}.iagd

    #setup
    cp $datapath/$src $tmp

    #run tests
    $binpath/iptctest $tmp <<-eoc
		a Iptc.Application2.Headline          The headline I am
		a Iptc.Application2.Keywords          Yet another keyword
		m Iptc.Application2.DateCreated       2004-8-3
		a Iptc.Application2.Urgency           3
		m Iptc.Application2.SuppCategory      "bla bla ba"
		a Iptc.Envelope.ModelVersion          2
		a Iptc.Envelope.TimeSent              14:41:0-05:00
		a Iptc.Application2.RasterizedCaption 230 42 34 2 90 84 23 146
eoc
    $binpath/iptcprint $tmp > $test

    #check results
    diffCheck $test $good
    echo -n "."
    rm $tmp
}

extendedTest()
{
    src=$1
    tmp="temp"
    test=${src}.ixtst
    good=$datapath/${src}.ixgd

    #setup
    cp $datapath/$src $tmp

    #run tests
    $binpath/iptctest $tmp < $datapath/ext.dat
    $binpath/iptcprint $tmp > $test

    #check results
    diffCheck $test $good
    echo -n "."
    rm $tmp
}


# Make sure to pass the test file first and the known good file second
diffCheck() 
{
    test=$1
    good=$2
    
    #run diff and check results
    diff -q $diffargs $test $good
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
binpath="../../src"
datapath="../data"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

test_files="smiley1.jpg smiley2.jpg glider.exv table.jpg"

errors=0
cd ./tmp
echo

echo -n "Read tests"
for i in $test_files; do printTest $i; done

echo
echo -n "Remove tests"
for i in $test_files; do removeTest $i; done

echo
echo -n "Add/Mod tests"
for i in $test_files; do addModTest $i; done

echo
echo -n "Extended tests"
for i in $test_files; do extendedTest $i; done

echo -e "\n---------------------------------------------------------"
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi

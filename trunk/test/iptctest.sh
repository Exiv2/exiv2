#! /bin/sh
# Test driver for Iptc metadata

printTest()
{
    src=$1
    test=${src}.iptst
    good=$datapath/${src}.ipgd

    #run tests
    $samples/iptcprint $datapath/$src > $test

    #check results
    diffCheck $test $good
    printf "."
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
    $samples/iptctest $tmp <<-eoc
		r Iptc.Application2.Byline
		r Iptc.Application2.Caption
		r Iptc.Application2.Keywords
		r Iptc.Application2.Keywords
		r Iptc.Application2.Keywords
		r Iptc.Application2.CountryName
eoc
    $samples/iptcprint $tmp > $test

    #check results
    diffCheck $test $good
    printf "."
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
    $samples/iptctest $tmp <<-eoc
		a Iptc.Application2.Headline          The headline I am
		a Iptc.Application2.Keywords          Yet another keyword
		m Iptc.Application2.DateCreated       2004-8-3
		a Iptc.Application2.Urgency           3
		m Iptc.Application2.SuppCategory      "bla bla ba"
		a Iptc.Envelope.ModelVersion          2
		a Iptc.Envelope.TimeSent              14:41:0-05:00
		a Iptc.Application2.RasterizedCaption 230 42 34 2 90 84 23 146
eoc
    $samples/iptcprint $tmp > $test

    #check results
    diffCheck $test $good
    printf "."
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
    $samples/iptctest $tmp < $datapath/ext.dat
    $samples/iptcprint $tmp > $test

    #check results
    diffCheck $test $good
    printf "."
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

if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
datapath="../data"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

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

errors=0
cd ./tmp
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

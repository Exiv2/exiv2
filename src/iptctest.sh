#! /bin/bash
# Test driver for IPTC metadata

printTest()
{
    src=$1
    test=${src}.iptst
    good=${src}.ipgd

    #run tests
    ../src/iptcprint $src > $test

    #check results
    diffCheck $test $good
    echo -n "."
}

removeTest()
{
    src=$1
    tmp="temp"
    test=${src}.irtst
    good=${src}.irgd

    #setup
    cp $src $tmp

    #run tests
    ../src/iptctest $tmp <<-eoc
		r Iptc.Application2.Byline
		r Iptc.Application2.Caption
		r Iptc.Application2.Keywords
		r Iptc.Application2.Keywords
		r Iptc.Application2.Keywords
		r Iptc.Application2.CountryName
eoc
    ../src/iptcprint $tmp > $test

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
    good=${src}.iagd

    #setup
    cp $src $tmp

    #run tests
    ../src/iptctest $tmp <<-eoc
		a Iptc.Application2.Headline          The headline I am
		a Iptc.Application2.Keywords          Yet another keyword
		m Iptc.Application2.DateCreated       2004-8-3
		a Iptc.Application2.Urgency           3
		m Iptc.Application2.SuppCategory      "bla bla ba"
		a Iptc.Envelope.ModelVersion          2
		a Iptc.Envelope.TimeSent              14:41:0-05:00
		a Iptc.Application2.RasterizedCaption 230 42 34 2 90 84 23 146
eoc
    ../src/iptcprint $tmp > $test

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
    good=${src}.ixgd

    #setup
    cp $src $tmp

    #run tests
    ../src/iptctest $tmp < ext.dat
    ../src/iptcprint $tmp > $test

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
    diff -q --strip-trailing-cr $test $good
    if [ $? -ne 0 ]; then
       let ++errors
    else
       rm $test
    fi 
}

test_files="smiley1.jpg smiley2.jpg glider.exv table.jpg"

let errors=0
cd ../test
echo

echo -n "Read tests"
for i in $test_files; do printTest $i; done

echo -ne "\nRemove tests"
for i in $test_files; do removeTest $i; done

echo -ne "\nAdd/Mod tests"
for i in $test_files; do addModTest $i; done

echo -ne "\nExtended tests"
for i in $test_files; do extendedTest $i; done

echo -e "\n---------------------------------------------------------"
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi

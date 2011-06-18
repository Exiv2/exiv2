#! /bin/sh
# Test driver for EPS files

export LC_ALL=C

exiv2="$VALGRIND ../../src/exiv2"

diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

(
    cd data/eps
    for epsfile in *.eps; do
        cp "$epsfile" ../../tmp/
        (
            cd ../../tmp

            echo "Command: exiv2 -pp $epsfile"
            $exiv2 -pp "$epsfile"
            echo "Exit code: $?"
            echo

            echo "Command: exiv2 -pa $epsfile"
            $exiv2 -pa "$epsfile"
            echo "Exit code: $?"
            echo
        )
    done
) > tmp/eps-test.out 2>&1

if diff -q $diffargs data/eps/eps-test.out tmp/eps-test.out ; then
    echo "All testcases passed."
else
    diff -u $diffargs data/eps/eps-test.out tmp/eps-test.out
fi

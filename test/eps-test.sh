#! /bin/sh
# Test driver for EPS files

export LC_ALL=C

cd tmp/

exiv2="$VALGRIND ../../src/exiv2"

exiv2version="`$exiv2 -V | sed -n '1 s,^exiv2 \([^ ]*\).*,\1,p'`"
if [ -z "$exiv2version" ]; then
    echo "Error: Unable to determine Exiv2 version"
    exit 1
fi

diffargs="--strip-trailing-cr"
if ! diff -q $diffargs /dev/null /dev/null 2>/dev/null ; then
    diffargs=""
fi

(
    for eps_image in ../data/eps/eps-*.eps; do
        image="`basename "$eps_image" .eps`"

        printf "." >&3

        cp "../data/eps/$image.eps" ./

        echo "Command: exiv2 -pp $image.eps"
        $exiv2 -pp "$image.eps"
        echo "Exit code: $?"
        echo

        echo "Command: exiv2 -pa $image.eps"
        $exiv2 -pa "$image.eps"
        echo "Exit code: $?"
        echo

        echo "Command: exiv2 -f -eX $image.eps"
        $exiv2 -f -eX "$image.eps"
        echo "Exit code: $?"
        echo

        if [ -e "$image.xmp" ] ; then
            echo "Command: diff -q ../data/eps/$image.xmp $image.xmp"
            diff -q "../data/eps/$image.xmp" "$image.xmp"
            echo "Exit code: $?"
            echo
        fi

        # Using "-ix" instead of "-iX" because the latter
        # executes writeMetadata() twice, making it hard to debug.

        cp "../data/eps/eps-test-newxmp.xmp" "$image.exv"
        sed "s,@Exiv2Version@,$exiv2version," < "../data/eps/$image.eps.newxmp" > "$image.eps.newxmp"

        echo "Command: exiv2 -ix $image.eps"
        $exiv2 -ix "$image.eps"
        echo "Exit code: $?"
        echo

        echo "Command: diff -q $image.eps.newxmp $image.eps"
        diff -q "$image.eps.newxmp" "$image.eps"
        echo "Exit code: $?"
        echo

        echo "Command: (2) exiv2 -ix $image.eps"
        $exiv2 -ix "$image.eps"
        echo "Exit code: $?"
        echo

        echo "Command: (2) diff -q $image.eps.newxmp $image.eps"
        diff -q "$image.eps.newxmp" "$image.eps"
        echo "Exit code: $?"
        echo
    done
) 3>&1 > "eps-test.out" 2>&1

echo "."

if ! diff -q $diffargs "../data/eps/eps-test.out" "eps-test.out" ; then
    diff -u $diffargs "../data/eps/eps-test.out" "eps-test.out"
    exit 1
fi

echo "All testcases passed."

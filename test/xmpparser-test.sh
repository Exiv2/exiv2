#!/usr/bin/env bash
# XMP parser test driver

source ./functions.source
if [ "$PLATFORM" == "NetBSD" -o "$PLATFORM" == "SunOS" -o "$PLATFORM" == "FreeBSD" ]; then
	echo "UNIX platform $PLATFORM Skip $0"
	exit 0 # UNIX diff complains about newline stuff
fi

##
# Check if xmpparser-test exists
if [ $(existsTest xmpparser-test) != 1 ] ; then
    echo "xmpparser-test not found. Assuming XMP support is not enabled."
    exit 0
fi

(   cd "$testdir"

    files=(BlueSquare.xmp StaffPhotographer-Example.xmp xmpsdk.xmp)
    copyTestFiles ${files[@]}

    for f in ${files[@]} ; do
        runTest xmpparser-test $f
        diff $diffargs $f ${f}-new
    done

    testfile=xmpsdk.xmp
    runTest xmpparse ${testfile} > t1 2>&1
    runTest xmpparse ${testfile}-new > t2 2>&1
    diff $diffargs t1 t2

    # ----------------------------------------------------------------------
    # xmpsample
    runTest xmpsample

    # ----------------------------------------------------------------------
    # XMP sample commands
    copyTestFiles exiv2-empty.jpg cmdxmp.txt
    runTest exiv2 -v -m cmdxmp.txt exiv2-empty.jpg
    runTest exiv2 -v -px exiv2-empty.jpg

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##

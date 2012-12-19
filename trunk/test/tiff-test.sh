#!/bin/bash
# TIFF parser test driver

# ----------------------------------------------------------------------
# Check if the exifprobe tool is available

if [ `which exifprobe 2>/dev/null`x = x ] ; then
    echo "tiff-test.sh: exifprobe not found. Skipping TIFF tests."
    exit 0
fi

# ----------------------------------------------------------------------
# Setup
source ./functions.source

(	cd "$testdir"

	testfile=mini9.tif
	copyTestFile ${testfile}
	exifprobe ${testfile}
	runTest tiff-test ${testfile}
	exifprobe ${testfile}

) > $results

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good

# That's all Folks!
##
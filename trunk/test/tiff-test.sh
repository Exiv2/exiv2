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

	copyTestFile mini9.tif
	runTest      mini9.tif
	exifprobe   $testfile

) > $results

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good

# That's all Folks!
##
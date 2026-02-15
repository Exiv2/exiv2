#!/usr/bin/env bash
# Test driver for tests of stringToLong/Float/Rational
source ./functions.source

(	cd "$testdir"

	runTest stringto-test

) > $results

reportTest

# That's all Folks!
##
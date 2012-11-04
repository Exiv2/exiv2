#!/bin/bash
# Test driver for tests of stringToLong/Float/Rational
source ./functions.source
prepareTest

(
	runTest stringto-test
) > $results

reportTest

# That's all Folks!
##
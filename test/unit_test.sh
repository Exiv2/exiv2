#!/usr/bin/env bash
# driver for $ make unit_test

( source ./functions.source ; runTest unit_tests --gtest_color=no 2>&1 | grep -v "Warning: Unsupported .... format" )

# That's all Folks!
##

#!/usr/bin/env bash
# Test driver for exiv2.exe --verbose --version

( source ./functions.source ; runTest unit_tests --gtest_color=no | grep -v "Warning: Unsupported date format")

# That's all Folks!
##

#!/bin/bash

# Debian & derivatives don't provide binary packages of googletest
# => have to build them ourselves
#
# This script builds a shared library of googletest (not googlemock!) and copies
# it to usr/lib/

mkdir gtest_build && cd gtest_build
cmake -DBUILD_SHARED_LIBS=1 /usr/src/googletest/googletest
make
cp libgtest* /usr/lib/
cd ..

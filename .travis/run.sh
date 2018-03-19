#!/bin/bash

set -e
set -x

mkdir test/tmp/
mkdir build && cd build
cmake ${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=install ..
make -j2
make tests
make install

cd ../tests/
python3 runner.py

#!/bin/bash

set -e
set -x

mkdir build && cd build
cmake ${CMAKE_OPTIONS} ..
cmake -DCMAKE_INSTALL_PREFIX=install ..
make -j
make tests
make install

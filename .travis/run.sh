#!/bin/bash

set -e
set -x

mkdir build && cd build
cmake ${CMAKE_OPTIONS} ..
make -j

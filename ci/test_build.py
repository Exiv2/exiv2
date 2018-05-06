#!/usr/bin/python3
# -*- coding: utf-8 -*-

import itertools
import multiprocessing
import os
import shlex
import subprocess
import sys

#: -DEXIV2_BUILD_SHARED_LIBS options
SHARED_LIBS = ["ON", "OFF"]

#: C & C++ compiler as tuples
CCS = ["gcc", "clang"]

#: -DCMAKE_BUILD_TYPE options
BUILD_TYPES = ["Debug", "Release"]

#: Additional parameters for cmake
CMAKE_OPTIONS = os.getenv("CMAKE_OPTIONS") or \
    "-DEXIV2_TEAM_EXTRA_WARNINGS=ON -DEXIV2_ENABLE_VIDEO=ON "\
    "-DEXIV2_ENABLE_WEBREADY=ON -DEXIV2_BUILD_UNIT_TESTS=ON"

#: cpu count
NCPUS = multiprocessing.cpu_count()


def call_wrapper(*args, **kwargs):
    """
    Wrapper around subprocess.call which terminates the program on non-zero
    return value.
    """
    return_code = subprocess.call(*args, **kwargs)
    if return_code != 0:
        sys.exit(return_code)


os.mkdir("build")

for params in itertools.product(SHARED_LIBS, CCS, BUILD_TYPES):

    lib_type, cc, build_type = params

    cxx = {"gcc": "g++", "clang": "clang++"}[cc]

    cwd = os.path.join("build", "_".join(params))
    os.mkdir(cwd)

    cmake = "cmake {!s} -DCMAKE_BUILD_TYPE={build_type} "\
        "-DBUILD_SHARED_LIBS={lib_type} ../.."\
        .format(CMAKE_OPTIONS, build_type=build_type, lib_type=lib_type)
    make = "make -j " + str(NCPUS)
    make_tests = "make tests"

    # set up environment
    env_copy = os.environ.copy()
    env_copy["CC"] = cc
    env_copy["CXX"] = cxx
    # env_copy["CFLAGS"] = flags
    # env_copy["CXXFLAGS"] = flags

    # location of the binaries for the new test suite:
    env_copy["EXIV2_PATH"] = "../" + cwd + "/bin"

    kwargs = {"env": env_copy, "cwd": cwd}

    call_wrapper(shlex.split(cmake), **kwargs)
    call_wrapper(shlex.split(make), **kwargs)
    call_wrapper(shlex.split(make_tests), **kwargs)

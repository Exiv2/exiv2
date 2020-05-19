#!/usr/bin/python3
# -*- coding: utf-8 -*-

import itertools
import multiprocessing
import os
import shlex
import subprocess
import sys


def call_wrapper(*args, **kwargs):
    """
    Wrapper around subprocess.call which terminates the program on non-zero
    return value.
    """
    return_code = subprocess.call(*args, **kwargs)
    if return_code != 0:
        sys.exit(return_code)


def matrix_build(shared_libs, ccs, build_types, cmake_bin, cmake_options,
                 tests=True):

    NCPUS = multiprocessing.cpu_count()

    os.mkdir("build")

    for params in itertools.product(shared_libs, ccs, build_types):

        lib_type, cc, build_type = params

        cwd = os.path.abspath(
            os.path.join(
                "build",
                "_".join(
                    map(lambda p: str(p) if p is not None else "", params)
                )
            )
        )
        os.mkdir(cwd)

        cmake = "{cmake_bin} {!s} -DCMAKE_BUILD_TYPE={build_type} -DCMAKE_CXX_FLAGS=-Wno-deprecated " \
            "-DBUILD_SHARED_LIBS={lib_type} -DEXIV2_BUILD_UNIT_TESTS={tests} -DCMAKE_CXX_STANDARD=98"\
            "../..".format(
                cmake_options, cmake_bin=cmake_bin, build_type=build_type,
                lib_type=lib_type, tests="ON" if tests else "OFF"
            )
        make = "make -j " + str(NCPUS)
        make_tests = "make tests"
        unit_test_binary = os.path.join(cwd, "bin", "unit_tests")

        # set compiler via environment only when requested
        env_copy = os.environ.copy()
        if cc is not None:
            cxx = {"gcc": "g++", "clang": "clang++"}[cc]
            env_copy["CC"] = cc
            env_copy["CXX"] = cxx

        # location of the binaries for the new test suite:
        env_copy["EXIV2_BINDIR"] = os.path.join(cwd, "bin")

        kwargs = {"env": env_copy, "cwd": cwd}

        def run(cmd):
            call_wrapper(shlex.split(cmd), **kwargs)

        run(cmake)
        run(make)
        if tests:
            run(make_tests)
            run(unit_test_binary)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(
        description="Build and test exiv2 using a matrix of build switches")
    parser.add_argument(
        "--compilers",
        help="Compilers to be used to build exiv2 (when none ore specified, "
        "then the default compiler will be used)",
        nargs='*',
        default=["gcc", "clang"],
        type=str
    )
    parser.add_argument(
        "--shared-libs",
        help="Values for the -DBUILD_SHARED_LIBS option",
        nargs='+',
        default=["ON", "OFF"],
        type=str
    )
    parser.add_argument(
        "--build-types",
        help="Values for the -DCMAKE_BUILD_TYPE option",
        nargs='+',
        default=["Debug", "Release"],
        type=str
    )
    parser.add_argument(
        "--cmake-executable",
        help="alternative name or path for the cmake executable",
        nargs=1,
        default=['cmake'],
        type=str
    )
    parser.add_argument(
        "--without-tests",
        help="Skip building and running tests",
        action='store_true'
    )
    parser.add_argument(
        "--cmake-options",
        help="Additional flags for cmake",
        type=str,
        nargs='?',
        default="-DEXIV2_TEAM_EXTRA_WARNINGS=ON -DEXIV2_ENABLE_VIDEO=ON "
        "-DEXIV2_ENABLE_WEBREADY=ON -DEXIV2_BUILD_UNIT_TESTS=ON "
        "-DBUILD_WITH_CCACHE=ON -DEXIV2_ENABLE_CURL=ON"
    )

    args = parser.parse_args()

    if len(args.compilers) == 0:
        args.compilers = [None]

    matrix_build(
        args.shared_libs, args.compilers, args.build_types,
        args.cmake_executable[0], args.cmake_options,
        not args.without_tests
    )

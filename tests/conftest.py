# -*- coding: utf-8 -*-

import os
import shutil

import system_tests


def pytest_addoption(parser):
    parser.addoption(
        "--suite-config",
        default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "suite.conf"),
        help="Path to the suite's configuration file",
    )
    parser.addoption(
        "--exiv2-debug",
        action="store_true",
        help="Print the commands that the system tests run, with expected and obtained output",
    )


def pytest_configure(config):
    # CaseMeta expands variables when a test module is imported, so the suite
    # has to be configured before collection starts. The tests use fixed file
    # names in $tmp_path, so every pytest process (each xdist worker, each
    # ctest/meson test running concurrently) gets its own subdirectory.
    system_tests.set_debug_mode(config.getoption("exiv2_debug"))
    system_tests.configure_suite(
        config.getoption("suite_config"),
        tmp_subdir=f"pytest-{os.getpid()}",
    )


def pytest_sessionfinish(session, exitstatus):
    # Keep the files of a failed run around for inspection
    if exitstatus == 0:
        os.chdir(session.config.rootpath)
        shutil.rmtree(system_tests.tmp_path, ignore_errors=True)

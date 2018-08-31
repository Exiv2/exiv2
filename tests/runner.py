# -*- coding: utf-8 -*-

if __name__ == '__main__':

    import argparse
    import os
    import unittest
    import sys

    import system_tests

    parser = argparse.ArgumentParser(description="The system test suite")

    parser.add_argument(
        "--config_file",
        type=str,
        nargs=1,
        help="Path to the suite's configuration file",
        default=['suite.conf']
    )
    parser.add_argument(
        "--verbose", "-v",
        action='count',
        help="verbosity level",
        default=1
    )
    parser.add_argument(
        "--debug",
        help="enable debugging output",
        action='store_true'
    )

    parser.add_argument(
        "dir_or_file",
        help="root directory under which the testsuite searches for tests or a"
        "single file which tests are run (defaults to the config file's"
        "location)",
        default=None,
        type=str,
        nargs='?'
    )

    args = parser.parse_args()
    conf_file = args.config_file[0]
    DEFAULT_ROOT = os.path.abspath(os.path.dirname(conf_file))

    system_tests.set_debug_mode(args.debug)
    system_tests.configure_suite(conf_file)

    if args.dir_or_file is None or os.path.isdir(args.dir_or_file):
        discovered_tests = unittest.defaultTestLoader.discover(
            args.dir_or_file or DEFAULT_ROOT
        )
    elif os.path.isfile(args.dir_or_file):
        discovered_tests = unittest.defaultTestLoader.discover(
            os.path.dirname(args.dir_or_file),
            pattern=os.path.split(args.dir_or_file)[1],
        )
    else:
        print(
            "WARNING: Invalid search location, falling back to {!s}"
            .format(DEFAULT_ROOT),
            file=sys.stderr
        )
        discovered_tests = unittest.defaultTestLoader.discover(
            DEFAULT_ROOT
        )

    test_res = unittest.runner.TextTestRunner(verbosity=args.verbose)\
                              .run(discovered_tests)

    sys.exit(0 if len(test_res.failures) + len(test_res.errors) == 0 else 1)

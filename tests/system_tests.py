# -*- coding: utf-8 -*-

import configparser
import os
import inspect
import subprocess
import threading
import shlex
import sys
import shutil
import string
import unittest


if sys.platform == 'win32':
    def _cmd_splitter(cmd):
        return cmd

    def _process_output_post(output):
        return output.replace('\r\n', '\n')

else:
    def _cmd_splitter(cmd):
        return shlex.split(cmd)

    def _process_output_post(output):
        return output


def _disjoint_dict_merge(d1, d2):
    """
    Merges two dictionaries with no common keys together and returns the result.

    >>> d1 = {"a": 1}
    >>> d2 = {"b": 2, "c": 3}
    >>> _disjoint_dict_merge(d1, d2) == {"a": 1, "b": 2, "c": 3}
    True

    Calling this function with dictionaries that share keys raises a ValueError:
    >>> _disjoint_dict_merge({"a": 1, "b": 6}, {"b": 2, "a": 3})
    Traceback (most recent call last):
     ..
    ValueError: Dictionaries have common keys.

    """
    inter = set(d1.keys()).intersection(set(d2.keys()))
    if len(inter) > 0:
        raise ValueError("Dictionaries have common keys.")
    res = d1.copy()
    res.update(d2)
    return res


#: global parameters extracted from the test suite's configuration file
_parameters = {}


def configure_suite(config_file):
    """
    Populates a global datastructure with the parameters from the suite's
    configuration file.

    This function performs the following steps:
    1. read in the file ``config_file`` via the ConfigParser module using
       extended interpolation
    2. check that the sections ``variables`` and ``paths`` are disjoint
    3. extract the environment variables given in the ``ENV`` section
    4. save all entries from the ``variables`` section in the global
       datastructure
    5. interpret all entries in the ``paths`` section as relative paths from the
       configuration file, expand them to absolute paths and save them in the
       global datastructure

    For further information concerning the rationale behind this, please consult
    the documentation in ``doc.md``.
    """

    if not os.path.exists(config_file):
        raise ValueError(
            "Test suite config file {:s} does not exist"
            .format(os.path.abspath(config_file))
        )

    config = configparser.ConfigParser(
        interpolation=configparser.ExtendedInterpolation(),
        delimiters=(':'),
        comment_prefixes=('#')
    )
    config.read(config_file)

    _parameters["suite_root"] = os.path.split(os.path.abspath(config_file))[0]
    _parameters["timeout"] = config.getfloat("General", "timeout", fallback=1.0)

    if 'variables' in config and 'paths' in config:
        intersecting_keys = set(config["paths"].keys())\
                            .intersection(set(config["variables"].keys()))
        if len(intersecting_keys) > 0:
            raise ValueError(
                "The sections 'paths' and 'variables' must not share keys, "
                "but they have the following common key{:s}: {:s}"
                .format(
                    's' if len(intersecting_keys) > 1 else '',
                    ', '.join(k for k in intersecting_keys)
                )
            )

    # extract variables from the environment
    for key in config['ENV']:
        if key in config['ENV fallback']:
            fallback = config['ENV fallback'][key]
        else:
            fallback = ""
        config['ENV'][key] = os.getenv(config['ENV'][key]) or fallback

    if 'variables' in config:
        for key in config['variables']:
            _parameters[key] = config['variables'][key]

    if 'paths' in config:
        for key in config['paths']:
            rel_path = config['paths'][key]
            abs_path = os.path.abspath(
                os.path.join(_parameters["suite_root"], rel_path)
            )
            if not os.path.exists(abs_path):
                raise ValueError(
                    "Path replacement for {short}: {abspath} does not exist"
                    " (was expanded from {rel})".format(
                        short=key,
                        abspath=abs_path,
                        rel=rel_path)
                )
            _parameters[key] = abs_path


def _setUp_factory(old_setUp, *files):
    """
    Factory function that returns a setUp function suitable to replace the
    existing setUp of a unittest.TestCase. The returned setUp calls at first
    old_setUp(self) and then creates a copy of all files in *files with the
    name: fname.ext -> fname_copy.ext

    All file names in *files are at first expanded using self.expand_variables()
    and the path to the copy is saved in self._file_copies
    """
    def setUp(self):
        old_setUp(self)
        self._file_copies = []
        for f in files:
            expanded_fname = self.expand_variables(f)
            fname, ext = os.path.splitext(expanded_fname)
            new_name = fname + '_copy' + ext
            self._file_copies.append(
                shutil.copyfile(expanded_fname, new_name)
            )
    return setUp


def _tearDown_factory(old_tearDown):
    """
    Factory function that returns a new tearDown method to replace an existing
    tearDown method. It at first deletes all files in self._file_copies and then
    calls old_tearDown(self).
    This factory is intended to be used in conjunction with _setUp_factory
    """
    def tearDown(self):
        for f in self._file_copies:
            os.remove(f)
        old_tearDown(self)
    return tearDown


def CopyFiles(*files):
    """
    Decorator for subclasses of system_test.Case that automatically creates a
    copy of the files specified as the parameters to the decorator.

    Example:
    >>> @CopyFiles("{some_var}/file.txt", "{another_var}/other_file.png")
        class Foo(Case):
            pass

    The decorator will inject new setUp method that at first calls the already
    defined setUp(), then expands all supplied file names using
    Case.expand_variables and then creates copies by appending '_copy' before
    the file extension. The paths to the copies are stored in self._file_copies.

    The decorator also injects a new tearDown method that deletes all files in
    self._file_copies and then calls the original tearDown method.

    This function will also complain if it is called without arguments or
    without paranthesis, which is valid decorator syntax but is obviously a bug
    in this case.
    """
    if len(files) == 0:
        raise ValueError("No files to copy supplied.")
    elif len(files) == 1:
        if isinstance(files[0], type):
            raise UserWarning(
                "Decorator used wrongly, must be called with filenames in paranthesis"
            )

    def wrapper(cls):
        old_setUp = cls.setUp
        cls.setUp = _setUp_factory(old_setUp, *files)

        old_tearDown = cls.tearDown
        cls.tearDown = _tearDown_factory(old_tearDown)

        return cls

    return wrapper


def test_run(self):
    """
    This function reads in the members commands, retval, stdout, stderr and runs
    the `expand_variables` function on each. The resulting commands are then run
    using the subprocess module and compared against the expected values that
    were provided in the static members via `compare_stdout` and
    `compare_stderr`. Furthermore a threading.Timer is used to abort the
    execution if a configured timeout is reached.

    It is automatically added as a member function to each system test by the
    CaseMeta metaclass. This ensures that it is run by each system test
    **after** setUp() and setUpClass() were run.
    """
    for i, command, retval, stdout, stderr in zip(range(len(self.commands)),
                                                  self.commands,
                                                  self.retval,
                                                  self.stdout,
                                                  self.stderr):
        command, retval, stdout, stderr = map(
            self.expand_variables, [command, retval, stdout, stderr]
        )
        retval = int(retval)
        timeout = {"flag": False}

        proc = subprocess.Popen(
            _cmd_splitter(command),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=self.work_dir
        )

        def timeout_reached(timeout):
            timeout["flag"] = True
            proc.kill()

        t = threading.Timer(
            _parameters["timeout"], timeout_reached, args=[timeout]
        )
        t.start()
        got_stdout, got_stderr = proc.communicate()
        t.cancel()

        processed_stdout = _process_output_post(got_stdout.decode('utf-8'))
        processed_stderr = _process_output_post(got_stderr.decode('utf-8'))

        self.assertFalse(timeout["flag"] and "Timeout reached")
        self.compare_stdout(i, command, processed_stdout, stdout)
        self.compare_stderr(i, command, processed_stderr, stderr)
        self.assertEqual(retval, proc.returncode)


class Case(unittest.TestCase):
    """
    System test case base class, provides the functionality to interpret static
    class members as system tests.

    The class itself only provides utility functions and system tests need not
    inherit from it, as it is automatically added via the CaseMeta metaclass.
    """

    #: maxDiff set so that arbitrarily large diffs will be shown
    maxDiff = None

    @classmethod
    def setUpClass(cls):
        """
        This function adds the variable work_dir to the class, which is the path
        to the directory where the python source file is located.
        """
        cls.work_dir = os.path.dirname(inspect.getfile(cls))

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """
        Function to compare whether the expected & obtained stdout match.

        This function is automatically invoked by test_run with the following
        parameters:
        i - the index of the current command that is run in self.commands
        command - the command that was run
        got_stdout - the obtained stdout, post-processed depending on the
                     platform so that lines always end with \n
        expected_stdout - the expected stdout extracted from self.stdout

        The default implementation simply uses assertMultiLineEqual from
        unittest.TestCase. This function can be overridden in a child class to
        implement a custom check.
        """
        self.assertMultiLineEqual(expected_stdout, got_stdout)

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        """ Same as compare_stdout only for standard-error. """
        self.assertMultiLineEqual(expected_stderr, got_stderr)

    def expand_variables(self, unexpanded_string):
        """
        Expands all variables of the form ``$var`` in the given string using the
        dictionary `variable_dict`.

        The expansion itself is performed by the string's template module using
        via `safe_substitute`.
        """
        return string.Template(str(unexpanded_string))\
            .safe_substitute(**self.variable_dict)


class CaseMeta(type):
    """ System tests generation metaclass.

    This metaclass is performs the following tasks:

    1. Add the `test_run` function as a member of the test class
    2. Add the `Case` class as the parent class
    3. Expand all variables already defined in the class, so that any additional
       code does not have to perform this task

    Using a metaclass instead of inheriting from case has the advantage, that we
    can expand all variables in the strings before any test case or even the
    class constructor is run! Thus users will immediately see the expanded
    result. Also adding the `test_run` function as a direct member and not via
    inheritance enforces that it is being run **after** the test cases setUp &
    setUpClass (which oddly enough seems not to be the case in the unittest
    module where test functions of the parent class run before setUpClass of the
    child class).
    """

    def __new__(mcs, clsname, bases, dct):

        changed = False

        # expand all non-private variables by brute force
        # => try all expanding all elements defined in the current class until
        # there is no change in them any more
        keys = [key for key in list(dct.keys()) if not key.startswith('_')]
        while not changed:
            for key in keys:

                old_value = dct[key]

                # only try expanding strings and lists
                if isinstance(old_value, str):
                    new_value = string.Template(old_value).safe_substitute(
                        **_disjoint_dict_merge(dct, _parameters)
                    )
                elif isinstance(old_value, list):
                    # do not try to expand anything but strings in the list
                    new_value = [
                        string.Template(elem).safe_substitute(
                            **_disjoint_dict_merge(dct, _parameters)
                        )
                        if isinstance(elem, str) else elem
                        for elem in old_value
                    ]
                else:
                    continue

                if old_value != new_value:
                    changed = True
                    dct[key] = new_value

        dct['variable_dict'] = _disjoint_dict_merge(dct, _parameters)
        dct['test_run'] = test_run

        if Case not in bases:
            bases += (Case,)

        return super(CaseMeta, mcs).__new__(mcs, clsname, bases, dct)

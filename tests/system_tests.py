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


class CasePreservingConfigParser(configparser.ConfigParser):
    r""" ConfigParser where the keys are case sensitive.

    The default ConfigParser converts all options in the config file with their
    lowercase version. This class overrides the respective functions and
    preserves the case of keys.

    The default behavior of ConfigParser:
    >>> conf_string = "[Section1]\nKey = Value"
    >>> default_conf = configparser.ConfigParser()
    >>> default_conf.read_string(conf_string)
    >>> list(default_conf['Section1'].keys())
    ['key']

    This classes' behavior:
    >>> case_preserve = CasePreservingConfigParser()
    >>> case_preserve.read_string(conf_string)
    >>> list(case_preserve['Section1'].keys())
    ['Key']
    """

    def optionxform(self, option):
        return option


#: global parameters extracted from the test suite's configuration file
_parameters = {}


#: setting whether debug mode is enabled or not
_debug_mode = False


def set_debug_mode(debug):
    """ Enable or disable debug mode

    In debug mode the test suite will print out all commands that it runs, the
    expected output and the actually obtained output
    """
    global _debug_mode
    _debug_mode = debug


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

    config = CasePreservingConfigParser(
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


class FileDecoratorBase(object):
    """
    Base class for decorators that manipulate files for test cases.

    The decorator expects to be provided with at least one file path
    on construction. When called, it replaces the setUp() and
    tearDown() functions of the type it is called on with custom ones.

    The new setUp() function performs the following steps:
    - call the old setUp()
    - create a file list in the decorated class with the name stored in
      FILE_LIST_NAME (defaults to _files)
    - iterate over all files, performing:
        - expand the file's path via expand_variables (member function
          of the decorated class)
        - call self.setUp_file_action(expanded file name)
        - append the result to the file list in the decorated class

    The function self.setUp_file_action is provided by this class and
    is intended to be overridden by child classes to provide some
    functionality, like file copies.


    The new tearDown() function performs the following steps:
    - iterate over all files in the file list:
         - call self.tearDown_file_action(filename)
    - call the old tearDown() function

    The function self.tearDown_file_action can be overridden by child
    classes. The default version provided by this class simply deletes
    all files that are passed to it.


    Example
    -------

    We'll inherit from FileDecoratorBase and override the member
    functions setUp_file_action and tearDown_file_action:

    >>> class TestDecorator(FileDecoratorBase):
    ...     def setUp_file_action(self, f):
    ...         print("setUp_file_action with", f)
    ...         return f.capitalize()
    ...
    ...     def tearDown_file_action(self, f):
    ...         print("tearDown_file_action with", f)

    Then, we use that decorator to wrap a class mocking
    system_tests.Case:

    >>> @TestDecorator("one", "two", "three")
    ... class MockCase(object):
    ...     def setUp(self):
    ...         print("calling MockCase.setUp()")
    ...
    ...     def tearDown(self):
    ...         print("calling MockCase.tearDown()")
    ...
    ...     def expand_variables(self, var):
    ...         return var + "_file"

    >>> M = MockCase()

    setUp has been replaced by a the new version, but the old one is
    still called. The new setUp iterates over all parameters passed to
    the constructor of the decorator, passes them to expand_variables
    and then to setUp_file_action:
    >>> M.setUp()
    calling MockCase.setUp()
    setUp_file_action with one_file
    setUp_file_action with two_file
    setUp_file_action with three_file

    The tearDown() function works accordingly:
    >>> M.tearDown()
    tearDown_file_action with One_file
    tearDown_file_action with Two_file
    tearDown_file_action with Three_file
    calling MockCase.tearDown()

    Please note the capitalized "file" names (this is due to
    setUp_file_action returning f.capitalized()) and that the old
    tearDown is called after the new one runs.
    """

    #: Name of the attribute in the decorated child class where the list of
    #: files is stored
    FILE_LIST_NAME = '_files'

    def __init__(self, *files):
        """
        Constructor of FileDecoratorBase.

        To prevent accidental wrong usage, it raises an exception if
        it is not called as a decorator with parameters.

        Only the following syntax works for this decorator:
        >>> @FileDecoratorBase("test")
        ... class Test(unittest.TestCase):
        ...     pass

        Calling it without parameters or without parenthesis raises an
        exception:
        >>> @FileDecoratorBase()
        ... class Test(unittest.TestCase):
        ...     pass
        Traceback (most recent call last):
         ..
        ValueError: No files supplied.

        >>> @FileDecoratorBase
        ... class Test(unittest.TestCase):
        ...     pass
        Traceback (most recent call last):
         ..
        UserWarning: Decorator used wrongly, must be called with filenames in parenthesis
        """
        if len(files) == 0:
            raise ValueError("No files supplied.")
        elif len(files) == 1:
            if isinstance(files[0], type):
                raise UserWarning(
                    "Decorator used wrongly, must be called with "
                    "filenames in parenthesis"
                )

        self.files = files

    def new_setUp(self, old_setUp):
        """
        Returns a new setUp() function that can be used as a class
        member function (i.e. invoked via self.setUp()).

        Its functionality is described in this classes' docstring.
        """

        def setUp(other):
            old_setUp(other)
            if hasattr(other, self.FILE_LIST_NAME):
                raise TypeError(
                    "{!s} already has an attribute with the name {!s} which "
                    "would be overwritten by setUp()"
                    .format(other, self.FILE_LIST_NAME)
                )
            setattr(other, self.FILE_LIST_NAME, [])
            for f in self.files:
                expanded_fname = other.expand_variables(f)
                getattr(other, self.FILE_LIST_NAME).append(
                    self.setUp_file_action(expanded_fname)
                )
        return setUp

    def setUp_file_action(self, expanded_file_name):
        """
        This function is called on each file that is passed to the
        constructor during the call of the decorated class' setUp().

        Parameters:
        - expanded_file_name: the file's path expanded via
                              expand_variables from system_tests.Case

        Returns:
        This function should return a path that will be stored in the decorated
        class' file list (the name is given by the attribute
        FILE_LIST_NAME). The custom tearDown() function (that is returned by
        self.new_tearDown()) iterates over this list and invokes
        self.tearDown_file_action on each element in that list.
        E.g. if a child class creates file copies, that should be deleted after
        the test ran, then one would have to return the path of the copy, so
        that tearDown() can delete the copies.

        The default implementation does nothing.
        """
        pass

    def new_tearDown(self, old_tearDown):
        """
        Returns a new tearDown() function that can be used as a class
        member function.

        It's functionality is described in this classes' docstring.
        """

        def tearDown(other):
            for f in getattr(other, self.FILE_LIST_NAME):
                self.tearDown_file_action(f)
            old_tearDown(other)

        return tearDown

    def tearDown_file_action(self, f):
        """
        This function is called on each file in the decorated class'
        file list (that list is populated during setUp()).

        It can be used to perform cleanup operations after a test run.

        Parameters:
        - f: An element of the file list

        Returns:
        The return value is ignored

        The default implementation removes f.
        """
        os.remove(f)

    def __call__(self, cls):
        """
        Call operator for the usage as a decorator. It is
        automatically used by Python when this class is used as a
        decorator.

        Parameters:
        - cls: The decorated type. Must be a type

        Returns:
        - cls where the setUp and tearDown functions have been
          replaced by the functions that are returned by
          self.new_setUp() and self.new_tearDown()
        """
        if not isinstance(cls, type):
            raise ValueError("The decorator must be called on a type")
        old_setUp = cls.setUp
        cls.setUp = self.new_setUp(old_setUp)

        old_tearDown = cls.tearDown
        cls.tearDown = self.new_tearDown(old_tearDown)

        return cls


class CopyFiles(FileDecoratorBase):
    """
    Decorator for subclasses of system_test.Case that automatically creates a
    copy of the files specified as the parameters passed to the decorator.

    Example:
    >>> @CopyFiles("$some_var/file.txt", "$another_var/other_file.png")
    ... class Foo(Case):
    ...     pass

    The decorator will inject a new setUp method that at first calls the
    already defined setUp(), then expands all supplied file names using
    Case.expand_variables and then creates copies by appending '_copy' before
    the file extension. The paths to the copies are stored in
    self._copied_files.

    The decorator also injects a new tearDown method that deletes all files in
    self._files and then calls the original tearDown method.

    This function will also complain if it is called without arguments or
    without paranthesis, which is valid decorator syntax but is obviously a bug
    in this case as it can result in tests not being run without a warning.
    """

    #: override the name of the file list
    FILE_LIST_NAME = '_copied_files'

    def setUp_file_action(self, expanded_file_name):
        fname, ext = os.path.splitext(expanded_file_name)
        new_name = fname + '_copy' + ext
        return shutil.copyfile(expanded_file_name, new_name)


class DeleteFiles(FileDecoratorBase):
    """
    Decorator for subclasses of system_test.Case that automatically deletes all
    files specified as the parameters passed to the decorator after the test
    were run.

    Example:
    >>> @DeleteFiles("$some_var/an_output_file", "auxiliary_output.bin")
    ... class Foo(Case):
    ...     pass

    The decorator injects new setUp() and tearDown() functions. The new setUp()
    at first calls the old setUp() and then saves all files that should be
    deleted later in self._files_to_delete. The new tearDown() actually deletes
    all files supplied to the decorator and then runs the original tearDown()
    function.
    """

    #: override the name of the file list
    FILE_LIST_NAME = '_files_to_delete'

    def setUp_file_action(self, expanded_file_name):
        return expanded_file_name


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
    if not (len(self.commands) == len(self.retval)
            == len(self.stdout) == len(self.stderr)):
        raise ValueError(
            "commands, retval, stdout and stderr don't have the same length"
        )
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

        if _debug_mode:
            print(
                '', "="*80, "will run: " + command, "expected stdout:", stdout,
                "expected stderr:", stderr,
                "expected return value: {:d}".format(retval),
                sep='\n'
            )

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

        processed_stdout = None
        processed_stderr = None
        for encoding in self.encodings:
            try:
                processed_stdout = _process_output_post(
                    got_stdout.decode(encoding)
                )
                processed_stderr = _process_output_post(
                    got_stderr.decode(encoding)
                )
            except UnicodeError:
                pass
            else:
                break
        if processed_stdout is None or processed_stderr is None:
            raise ValueError(
                "Could not decode the output of the command '{!s}' with the "
                "following encodings: {!s}"
                .format(command, ','.join(self.encodings))
            )

        if _debug_mode:
            print(
                "got stdout:", processed_stdout, "got stderr:",
                processed_stderr, "got return value: {:d}"
                .format(proc.returncode),
                sep='\n'
            )

        self.assertFalse(timeout["flag"], msg="Timeout reached")
        self.compare_stdout(i, command, processed_stdout, stdout)
        self.compare_stderr(i, command, processed_stderr, stderr)
        self.assertEqual(
            retval, proc.returncode, msg="Return value does not match"
        )


class Case(unittest.TestCase):
    """
    System test case base class, provides the functionality to interpret static
    class members as system tests.

    The class itself only provides utility functions and system tests need not
    inherit from it, as it is automatically added via the CaseMeta metaclass.
    """

    #: maxDiff set so that arbitrarily large diffs will be shown
    maxDiff = None

    #: list of encodings that are used to decode the test program's output
    #: the first encoding that does not raise a UnicodeError is used
    encodings = ['utf-8', 'iso-8859-1']

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
        self.assertMultiLineEqual(
            expected_stdout, got_stdout, msg="Standard output does not match"
        )

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        """ Same as compare_stdout only for standard-error. """
        self.assertMultiLineEqual(
            expected_stderr, got_stderr, msg="Standard error does not match"
        )

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

        changed = True

        # expand all non-private variables by brute force
        # => try all expanding all elements defined in the current class until
        # there is no change in them any more
        keys = [key for key in list(dct.keys()) if not key.startswith('_')]
        while changed:
            changed = False

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


def check_no_ASAN_UBSAN_errors(self, i, command, got_stderr, expected_stderr):
    """
    Drop-in replacement for the default Case.compare_stderr() function that
    **only** checks for any signs of ASAN (address sanitizer) and UBSAN
    (undefined behavior sanitizer).

    Parameters:
    - i, command, expected_stderr: ignored
    - got_stderr: checked for signs of ASAN und UBSAN error messages

    This function ignores the expected output to stderr! It is intended for
    test cases where standard error is filled with useless debugging
    messages/warnings that are not really relevant for the test and not worth
    storing in the test suite. This function can be used to still be able to
    catch ASAN & UBSAN error messages.

    Example usage
    -------------

    Override the default compare_stderr function in your subclass of Case with
    this function:
    >>> class TestNoAsan(Case):
    ...     compare_stderr = check_no_ASAN_UBSAN_errors

    >>> T = TestNoAsan()

    The new compare_stderr will only complain if there are strings inside the
    obtained stderr which could be an error reported by ASAN/UBSAN:
    >>> T.compare_stderr(0, "", "runtime error: load of value 190", "some output")
    Traceback (most recent call last):
     ..
    AssertionError: 'runtime error' unexpectedly found in 'runtime error: load of value 190'

    >>> T.compare_stderr(0, "", "SUMMARY: AddressSanitizer: heap-buffer-overflow", "")
    Traceback (most recent call last):
     ..
    AssertionError: 'AddressSanitizer' unexpectedly found in 'SUMMARY: AddressSanitizer: heap-buffer-overflow'

    It will not complain in all other cases, especially when expected_stderr
    and got_stderr do not match:
    >>> T.compare_stderr(0, "", "some output", "other output")
    """
    self.assertNotIn("runtime error", got_stderr)
    self.assertNotIn("AddressSanitizer", got_stderr)

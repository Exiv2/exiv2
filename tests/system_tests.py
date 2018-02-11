# -*- coding: utf-8 -*-

import configparser
import os
import inspect
import subprocess
import threading
import shlex
import sys
import shutil
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
    - create a list _files in the decorated class
    - iterate over all files, performing:
        - expand the file's path via expand_variables (member function
          of the decorated class)
        - call self.setUp_file_action(expanded file name)
        - append the result to _files in the decorated class

    The function self.setUp_file_action is provided by this class and
    is intended to be overridden by child classes to provide some
    functionality, like file copies.


    The new tearDown() function performs the following steps:
    - iterate over all files in _files (from the decorated class):
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

        self._files = files

    def new_setUp(self, old_setUp):
        """
        Returns a new setUp() function that can be used as a class
        member function (i.e. invoked via self.setUp()).

        It's functionality is described in this classes' docstring.
        """

        def setUp(other):
            old_setUp(other)
            other._files = []
            for f in self._files:
                expanded_fname = other.expand_variables(f)
                other._files.append(
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
        This function should return a path that will be stored in the
        decorated class' list _files. The custom tearDown() function
        (that is returned by self.new_tearDown()) iterates over this
        list and invokes self.tearDown_file_action on each element in
        that list.
        E.g. if a child class creates file copies, that should be
        deleted after the test ran, then one would have to return the
        path of the copy, so that tearDown() can delete the copies.

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
            for f in other._files:
                self.tearDown_file_action(f)
            old_tearDown(other)

        return tearDown

    def tearDown_file_action(self, f):
        """
        This function is called on each file in the decorated class'
        list _files (that list is populated during setUp()).

        It can be used to perform cleanup operations after a test run.

        Parameters:
        - f: An element of _files

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
    >>> @CopyFiles("{some_var}/file.txt", "{another_var}/other_file.png")
    ... class Foo(Case):
    ...     pass

    The decorator will inject new setUp method that at first calls the already
    defined setUp(), then expands all supplied file names using
    Case.expand_variables and then creates copies by appending '_copy' before
    the file extension. The paths to the copies are stored in self._files.

    The decorator also injects a new tearDown method that deletes all files in
    self._files and then calls the original tearDown method.

    This function will also complain if it is called without arguments or
    without paranthesis, which is valid decorator syntax but is obviously a bug
    in this case as it can result in tests not being run without a warning.
    """

    def setUp_file_action(self, expanded_file_name):
        fname, ext = os.path.splitext(expanded_file_name)
        new_name = fname + '_copy' + ext
        return shutil.copyfile(expanded_file_name, new_name)


class Case(unittest.TestCase):
    """
    System test case base class, provides the functionality to interpret static
    class members as system tests and runs them.

    This class reads in the members commands, retval, stdout, stderr and runs
    the format function on each, where format is called with the kwargs being a
    merged dictionary of all variables that were extracted from the suite's
    configuration file and all static members of the current class.

    The resulting commands are then run using the subprocess module and compared
    against the expected values that were provided in the static
    members. Furthermore a threading.Timer is used to abort the execution if a
    configured timeout is reached.

    The class itself must be inherited from, otherwise it is not useful at all,
    as it does not provide any static members that could be used to run system
    tests. However, a class that inherits from this class needn't provide any
    member functions at all, the inherited test_run() function performs all
    required functionality in child classes.
    """

    """ maxDiff set so that arbitrarily large diffs will be shown """
    maxDiff = None

    @classmethod
    def setUpClass(cls):
        """
        This function adds the variables variable_dict & work_dir to the class.

        work_dir - set to the file where the current class is defined
        variable_dict - a merged dictionary of all static members of the current
                        class and all variables extracted from the suite's
                        configuration file
        """
        cls.variable_dict = _disjoint_dict_merge(cls.__dict__, _parameters)
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
        """
        Same as compare_stdout only for standard-error.
        """
        self.assertMultiLineEqual(expected_stderr, got_stderr)

    def expand_variables(self, string):
        """
        Expands all variables in curly braces in the given string using the
        dictionary variable_dict.

        The expansion itself is performed by the builtin string method format().
        A KeyError indicates that the supplied string contains a variable
        in curly braces that is missing from self.variable_dict
        """
        return str(string).format(**self.variable_dict)

    def test_run(self):
        """
        Actual system test function which runs the provided commands,
        pre-processes all variables and post processes the output before passing
        it on to compare_stderr() & compare_stdout().
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

            self.assertFalse(timeout["flag"] and "Timeout reached")
            self.compare_stdout(
                i, command,
                _process_output_post(got_stdout.decode('utf-8')), stdout
            )
            self.compare_stderr(
                i, command,
                _process_output_post(got_stderr.decode('utf-8')), stderr
            )
            self.assertEqual(retval, proc.returncode)

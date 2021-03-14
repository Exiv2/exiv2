<div id="TOC"/>

- [README-TESTS](#README-TESTS)
  - [Running the test suite](#running-the-test-suite)
  - [Writing new tests](#writing-new-tests)
    - [Based on unittest](#Based-on-unittest)
    - [Based on system_tests](#Based-on-system_tests)
  - [Test suite](#test-suite)
    - [Configuration](#configuration)
      - [INI style](#ini-style)
      - [Parameters](#parameters)
    - [Test cases](#test-cases)
    - [Multiline strings](#multiline-strings)
    - [Paths](#paths)
  - [Advanced test cases](#advanced-test-cases)
    - [Providing standard input to commands](#providing-standard-input-to-commands)
    - [Using a different output encoding](#using-a-different-output-encoding)
    - [Working with binary output](#working-with-binary-output)
    - [Setting and modifying environment variables](#setting-and-modifying-environment-variables)
    - [Creating file copies](#creating-file-copies)
    - [Customizing the output check](#customizing-the-output-check)
    - [Running all commands under valgrind](#running-all-commands-under-valgrind)
    - [Manually expanding variables in strings](#manually-expanding-variables)
    - [Hooks](#hooks)
    - [Possible pitfalls](#possible-pitfalls)
  - [Bash test cases](#bash-test-cases)


<div id="README-TESTS"/>
# README-TESTS

This test suite is intended for system tests, i.e. for running a binary with
certain parameters and comparing the output against an expected value. This is
especially useful for a regression test suite, but can be also used for testing
of new features where unit testing is not feasible, e.g. to test new command
line parameters.

<div id="running-the-test-suite"/>
## Running the test suite

The test suite is written for Python 3 and is not compatible with Python 2, thus
it must be run with `python3` and not with `python` (which is usually an alias
for Python 2).

Then navigate to the `tests/` subdirectory and run:

```bash
python3 -m pip install -r requirements.txt
python3 runner.py
```

One can supply the script with a directory where the suite should look for the
tests (it will search the directory recursively). If omitted, the runner will
look in the directory where the configuration file is located. It is also
possible to instead pass a file as the parameter, the test suite will then only
run the tests from this file.

The runner script also supports the optional arguments `--config_file` which
allows to provide a different test suite configuration file than the default
`suite.conf`. It also forwards the verbosity setting via the `-v`/`--verbose`
flags to Python's unittest module.

Optionally one can provide the `--debug` flag which will instruct test suite to
print all command invocations and all expected and obtained outputs to the
standard output.

[TOC](#TOC)

<div id="writing-new-tests"/>

## Writing new tests

The test suite is intended to run a binary and compare its standard output,
standard error and return value against provided values. This is implemented
using Python's [unittest](https://docs.python.org/3/library/unittest.html) module and thus all test files are Python files.

When creating new tests, follow roughly these steps:

1. Choose an appropriate subdirectory where the test belongs. If none fits
   create a new one and put an empty `__init__.py` file there.

2. Create a new file with a name matching `test_*.py`. Write test cases in it.

3. Run the test suite:
    ```sh
    python3 runner.py               # automatically find test scripts and execute them
                        [test.py]   # executes only the test script for the specified path
                        -v          # verbose output
    ```

<div id="Based-on-unittest"/>

### Based on unittest

You can write standard [unittest](https://docs.python.org/3/library/unittest.html) test cases. For example:
```py
import os
import unittest

from system_tests import BT                 # import system_tests, which has been loaded into sys.path


class TestCases(unittest.TestCase):

    def setUp(self):
        """ This function is executed before each test case. """
        os.makedirs(BT.Config.tmp_dir, exist_ok=True)
        os.chdir(BT.Config.tmp_dir)         # switch to the temporary directory to test
        BT.Config.init()

    def simple_test(self):                  # define a test function
        e = BT.Executer('exiv2 --version')  # execute a command in the shell
        assert e.returncode == 0:
        if 'exiv2 0.27.4.9' not in e.stdout:
            raise RuntimeError('Wrong version')

    def addmoddel_test(self):               # define another test function
        jpg      = 'exiv2-empty.jpg'
        BT.copyTestFile(jpg)
        out      = BT.Output()
        out     += BT.Executer('addmoddel {jpg}', vars())
        out     += BT.Executer('exiv2 -pv {jpg}', vars())
        BT.reportTest('addmoddel', out)

```
`system_tests.BT` is defined in `tests/utils/*.py`, which provides some functions and classes that are compatible with different platforms, making it easier for you to write test cases.

<div id="Based-on-system_tests"/>

### Base on system_tests

In addition to unittest test cases, you can also write a declarative test case by inheriting `system_tests.CaseMeta`. For example:
```python
# -*- coding: utf-8 -*-

import system_tests


class GoodTestName(metaclass=system_tests.CaseMeta):

    filename = "$data_path/test_file"
    commands = ["$exiv2 $filename", "$exiv2 $filename" + '_2']
    stdout = [""] * 2
    stderr = ["""$exiv2_exception_msg $filename:
$kerFailedToReadImageData
"""] * 2
    retval = [1] * 2
```

The test suite will run the provided commands in `commands` and compare them to
the output in `stdout` and `stderr` and it will compare the return values.

The strings after a `$` are variables either defined in this test's class or are
taken from the suite's configuration file (see `doc.md` for a complete
explanation).


[TOC](#TOC)

<div id="test-suite"/>
## Test suite

The test suite itself uses the builtin `unittest` module of Python to discover
and run the individual test cases. The test cases themselves are implemented in
Python source files, but the required Python knowledge is minimal.

The test suite is configured via one configuration file whose location
automatically sets the root directory of the test suite. The `unittest` module
then recursively searches all sub-directories with a `__init__.py` file for
files of the form `test_*.py`, which it automatically interprets as test cases
(more about these in the next section). Python will automatically interpret each
directory as a module and use this to format the output, e.g. the test case
`regression/crashes/test_bug_15.py` will be interpreted as the module
`regression.crashes.test_bug_15`. Thus one can use the directory structure to
group test cases.

### Configuration

<div id="ini-style"/>
#### INI style

The test suite is configured via `INI` style files using Python's builtin
[ConfigParser](https://docs.python.org/3/library/configparser.html)
module. Such a configuration file looks roughly like this:

```ini
[DEFAULT]
some_var: some_val

[section 1]
empty_var:
multiline_var: this is a multiline string
    as long as the indentation
    is present
# comments can be inserted
# some_var is implicitly present in this section by the DEFAULT section

[section 2]
# set some_var for this section to something else than the default
some_var: some_other_val
# values from other sections can be inserted
vars can have whitespaces: ${some_var} ${section 1: multiline var}
multiline var: multiline variables can have

    empty lines too
```

For further details concerning the syntax, please consult the official
documentation. The `ConfigParser` module is used with the following defaults:
- Comments are started by `#` only
- The separator between a variable and the value is `:`
- Multiline comments can have empty lines
- Extended Interpolation is used (this allows to refer to other sections when
  inserting values using the `${section:variable}` syntax)

Please keep in mind that leading and trailing whitespaces are **stripped** from
strings when extracting variable values. So this:

```ini
some_var:     some value with whitespaces before and after    
```
is equivalent to this:
```ini
some_var:some value with whitespaces before and after
```

[TOC](#TOC)

<div id="parameters"/>
#### Parameters

The test suite's configuration file should have the following form:

```ini
[General]
timeout: 0.1

[paths]
binary: ../build/bin/binary
important_file: ../conf/main.cfg

[variables]
abort_error: ERROR
abort_exit value: 1
```

The General section only contains the `timeout` parameter, which is actually
optional (when left out 1.0 is assumed). The timeout sets the maximum time in
seconds for each command that is run before it is aborted. This allows for test
driven development with tests that cause infinite loops or similar hangs in the
test suite.

The paths and variables sections define global variables for the system test
suite, which every test case can read. Following the DRY principle, one can put
common outputs of the tested binary in a variable, so that changing an error
message does not result in an hour long update of the test suite. Both sections
are merged together before being passed on to the test cases, thus they must not
contain variables with the same name (doing so results in an error).

While the values in the variables section are simply passed on to the test cases
the paths section is special as its contents are interpreted as relative paths
(with respect to the test suite's root) and are expanded to absolute paths
before being passed to the test cases. This can be used to inform each test case
about the location of a built binary or a configuration file without having to
rely on environment variables.

However, sometimes environment variables are very handy to implement variable
paths or platform differences (like different build directories or file
extensions). For this, the test suite supports the `ENV` and `ENV fallback`
sections. In conjunction with the extended interpolation of the `ConfigParser`
module, these can be quite useful. Consider the following example:

```ini
[General]
timeout: 0.1

[ENV]
variable_prefix: PREFIX
file_extension: FILE_EXT

[ENV fallback]
variable_prefix: ../build

[paths]
binary: ${ENV:variable_prefix}/bin/binary${ENV:file_extension}
important_file: ../conf/main.cfg

[variables]
abort_error: ERROR
abort_exit value: 1
```

The `ENV` section is, similarly to the `paths` section, special insofar as the
variables are extracted from the environment with the given name. E.g. the
variable `file_extension` would be set to the value of the environment variable
`FILE_EXT`. If the environment variable is not defined, then the test suite will
look in the `ENV fallback` section for a fallback. E.g. in the above example
`variable_prefix` has the fallback or default value of `../build` which will be
used if the environment variable `PREFIX` is not set. If no fallback is provided
then an empty string is used instead, which would happen to `file_extension` if
`FILE_EXT` would be unset.

This can be combined with the extended interpolation of Python's `ConfigParser`,
which allows to include variables from arbitrary sections into other variables
using the `${sect:var_name}` syntax. This would be expanded to the value of
`var_name` from the section `sect`. The above example only utilizes this in the
`paths` section, but it can also be used in the `variables` section, if that
makes sense for the use case.

Returning to the example config file, the path `binary` would be inferred in the
following steps:
1. extract `PREFIX` & `FILE_EXT` from the environment, if they don't exist use
   the default values from `ENV fallback` or ""
2. substitute the strings `${ENV:variable_prefix}` and `${ENV:file_extension}`
3. expand the relative path to an absolute path

Please note that while the `INI` file allows for variables with whitespaces or
`-` in their names, such variables will cause errors as they are invalid
variable names in Python.


[TOC](#TOC)

<div id="test-cases"/>
### Test cases

The test cases are defined in Python source files utilizing the unittest module,
thus every file must also be a valid Python file. Each file defining a test case
must start with `test_` and have the file extension `py`. To be discovered by
the unittest module it must reside in a directory with a (empty) `__init__.py`
file.

A test case should test one logical unit, e.g. test for regressions of a certain
bug or check if a command line option works. Each test case can run multiple
commands which results are compared to an expected standard output, standard
error and return value. Should differences arise or should one of the commands
take too long, then an error message with the exact differences is shown to the
user.

An example test case file would look like this:

```python
# -*- coding: utf-8 -*-

import system_tests


class AnInformativeName(metaclass=system_tests.CaseMeta):

    filename = "invalid_input_file"
    commands = [
	    "$binary -c $import_file -i $filename"
	]
    retval = ["$abort_exit_value"]
    stdout = ["Reading $filename"]
    stderr = [
        """$abort_error
error in $filename
"""
    ]
```

The first 6 lines are necessary boilerplate to pull in the necessary routines to
run the actual tests (these are implemented in the module `system_tests` with
the meta-class `system_tests.CaseMeta` which performs the necessary preparations
for the tests to run). When adding new tests one should choose a new class name
that briefly summarizes the test. Note that the file name (without the
extension) with the directory structure is interpreted as the module by Python
and pre-pended to the class name when reporting about the tests. E.g. the file
`regression/crashes/test_bug_15.py` with the class `OutOfBoundsRead` gets
reported as `regression.crashes.test_bug_15.OutOfBoundsRead` already including
a brief summary of this test.

In the following lines the lists `commands`, `retval`, `stdout` and `stderr`
should be defined. These are lists of strings and must all have the same number
of elements.

The test suite at first takes all these strings and substitutes all values
following a `$` with variables either defined in this class alongside (like
`filename` in the above example) or with the values defined in the test suite's
configuration file. Please note that defining a variable with the same name as a
variable in the suite's configuration file will result in an error (otherwise
one of the variables would take precedence leading to unexpected results). The
variables defined in the test suites configuration file are also available in
the `system_tests` namespace. In the above example it would be therefore
possible to access `abort_exit_value` via `system_tests.abort_exit_value`
(please be aware that all values will be strings though).

The substitution of values is performed using the template module from Python's
string library via `safe_substitute`. In the above example the command would
thus expand to:
```bash
/path/to/the/dir/build/bin/binary -c /path/to/the/dir/conf/main.cfg -i invalid_input_file
```
and similarly for `stdout` and `stderr`.

Once the substitution is performed, each command is run using Python's
`subprocess` module, its output is compared to the values in `stdout` and
`stderr` and its return value to `retval`. Please note that for portability
reasons the subprocess module is run with `shell=False`, thus shell expansions,
pipes and redirections into files will not work.

As the test cases are implemented in Python, one can take full advantage of
Python for the construction of the necessary lists. For example when 10 commands
should be run and all return 0, one can write `retval = 10 * [0]` instead of
writing 0 ten times. The same is of course possible for strings.


[TOC](#TOC)

<div id="multiline-strings"/>
### Multiline strings

It is generally recommended to use Python's multiline strings (strings starting
and ending with three `"` instead of one `"`) for the elements of the `commands`
list, especially when the commands include `"` or escape sequences. Proper
escaping is tricky to get right in a platform independent way, as it depends on
the terminal that is used. Using multiline strings circumvents this issue.

There are however some peculiarities with multiline strings in Python. Normal
strings start and end with a single `"` but multiline strings start with three
`"`. Also, while the variable names must be indented, new lines in multiline
strings must not or additional whitespaces will be added. E.g.:

```python
    stderr = [
        """something
        else"""
    ]
```
will actually result in the string:

```
something
        else
```
and not:
```
something
else
```
as the indentation might have suggested.

Also note that in this example the string will not be terminated with a newline
character. To achieve that put the `"""` on the following line.

[TOC](#TOC)

<div id="paths"/>
### Paths

Some test cases require the specification of paths (e.g. to the location of test
cases). This can be problematic when working with the Windows operating system,
as it sometimes exhibits problems with `/` as path separators instead of `\`,
which cannot be used on every other platform.

This can be circumvented by creating the paths via `os.path.join`, but that is
quite verbose. A slightly simpler alternative is the function `path` from
`system_tests` which converts all `/` inside your string into the platform's
default path separator:

```python
# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class AnInformativeName(metaclass=CaseMeta):

    filename = path("$path_to_test_files/invalid_input_file")

    # the rest of your test case
```


[TOC](#TOC)

<div id="advanced-test-cases"/>
## Advanced test cases

This section describes more advanced features that are probably not necessary
the "standard" usage of the test suite.


<div id="providing-standard-input-to-commands"/>
### Providing standard input to commands

The test suite supports providing a standard input to commands in a similar
fashion as the standard output and error are specified: it expects a list (with
the length equal to the number of commands) of standard inputs (either strings
or `bytes`). For commands that expect no standard input, simply set the
respective entry to `None`:

```python
# -*- coding: utf-8 -*-

import system_tests


class AnInformativeName(metaclass=system_tests.CaseMeta):

    commands = [
	    "$binary -c $import_file --",
        "$binary -c $import_file --"
	]
    retval = [1, 1]
    stdin = [
        "read file a",
        None
    ]
    stdout = [
        "Reading...",
        ""
    ]
    stderr = [
        "Error",
        "No input provided"
    ]
```

In this example, the command `$binary -c $import_file --` would be run twice,
first with the standard input `read file a` and second without any input
(resulting in the error `No input provided`).

If all commands don't expect any standard input, omit the attribute `stdin`, the
test suite will implicitly assume `None` for every command.


[TOC](#TOC)

<div id="using-a-different-output-encoding"/>
### Using a different output encoding

The test suite will try to interpret the program's output as utf-8 encoded
strings and if that fails it will try the `iso-8859-1` encoding (also know as
`latin-1`).

If the tested program outputs characters in another encoding then it can be
supplied as the `encodings` parameter in each test case:

```python
# -*- coding: utf-8 -*-

import system_tests


class AnInformativeName(metaclass=system_tests.CaseMeta):

    encodings = ['ascii']

    filename = "invalid_input_file"
    commands = [
	    "$binary -c $import_file -i $filename"
	]
    retval = ["$abort_exit_value"]
    stdout = ["Reading $filename"]
    stderr = [
        """$abort_error
error in $filename
"""
    ]
```

The test suite will try to decode the program's output with the provided
encodings in the order that they appear in the list. It will select the first
encoding that can decode the output successfully. If no encoding is able to
decode the program's output, then an error is raised. The list of all supported
encodings can be found
[here](https://docs.python.org/3/library/codecs.html#standard-encodings).


[TOC](#TOC)

<div id="working-with-binary-output"/>
### Working with binary output

Some programs output binary data directly to stdout or stderr. Such programs can
be also tested by specifying the type `bytes` as the only member in the
`encodings` list and supplying `stdout` and/or `stderr` as `bytes` and not as a
string.

An example test case would look like this:

```python
# -*- coding: utf-8 -*-

import system_tests


class AnInformativeName(metaclass=system_tests.CaseMeta):

    encodings = [bytes]

    commands = ["$prog --dump-binary"]
    retval = [1]
    stdout = [bytes([1, 2, 3, 4, 16, 42])]
    stderr = [bytes()]
```

Using the bytes encoding has the following limitations:
- variables of the form `$some_var` cannot be expanded in `stdout` and `stderr`
- if the `bytes` encoding is specified, then both `stderr` and `stdout` must be
  valid `bytes`


### Setting and modifying environment variables

The test suite supports setting or modifying environment variables for
individual test cases. This can be accomplished by adding a member dictionary
named `env` with the appropriate variable names and keys:

```python
# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class AnInformativeName(metaclass=CaseMeta):

    env = {
        "MYVAR": 26,
        "USER": "foobar"
    }

    # if you want a pristine environment, consisting only of MYVAR & USER,
    # uncomment the following line:
    # inherit_env = False

    # the rest of the test case follows
```

All commands belonging to this test case will be run with a modified environment
where the variables `MYVAR` and `USER` will be set to the specified
values. By default the environment is inherited from the user's environment and
the specified variables in `env` take precedence over the variables in the
user's environment (in the above example the variable `$USER` would be
overridden). If no variables should be inherited set `inherit_env` to `False`
and your test case will get only the specified environment variables.


[TOC](#TOC)

<div id="creating-file-copies"/>
### Creating file copies

For tests that modify their input file it is useful to run these with a
disposable copy of the input file and not with the original. For this purpose
the test suite features a decorator which creates a copy of the supplied files
and deletes the copies after the test ran.

Example:

```python
# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$filename", "$some_path/another_file.txt")
class AnInformativeName(metaclass=system_tests.CaseMeta):

    filename = "invalid_input_file"
    commands = [
	    "$binary -c $import_file -i $filename"
	]
    retval = ["$abort_exit_value"]
    stdout = ["Reading $filename"]
    stderr = [
        """$abort_error
error in $filename
"""
    ]
```

In this example, the test suite would automatically create a copy of the files
`invalid_input_file` and `$some_path/another_file.txt` (`some_path` would be of
course expanded too) named `invalid_input_file_copy` and
`$some_path/another_file_copy.txt`. After the test ran, the copies are
deleted. Please note that variable expansion in the filenames is possible.


[TOC](#TOC)

<div id="customizing-the-output-check"/>
### Customizing the output check

Some tests do not require a "brute-force" comparison of the whole output of a
program but only a very simple check (e.g. that a string is present). For these
cases, one can customize how stdout and stderr checked for errors.

The `system_tests.Case` class has two public functions for the check of stdout &
stderr: `compare_stdout` & `compare_stderr`. They have the following interface:

```python
compare_stdout(self, i, command, got_stdout, expected_stdout)
compare_stderr(self, i, command, got_stderr, expected_stderr)
```

with the parameters:
- i: index of the command in the `commands` list
- command: a string of the actually invoked command
- got_stdout/stderr: the obtained stdout, post-processed depending on the
  platform so that lines always end with `\n`
- expected_stdout/stderr: the expected output extracted from
  `self.stdout`/`self.stderr`

These functions can be overridden in child classes to perform custom checks (or
to omit them completely, too). Please however note, that it is not possible to
customize how the return value is checked. This is indented, as the return value
is often used by the OS to indicate segfaults and ignoring it (in combination
with flawed checks of the output) could lead to crashes not being noticed.

A drop-in replacement for `compare_stderr` is provided by the `system_tests`
module itself: `check_no_ASAN_UBSAN_errors`. This function only checks that
errors from AddressSanitizer and undefined behavior sanitizer are not present in
the obtained output to standard error **and nothing else**. This is useful for
test cases where stderr is filled with warnings that are not worth being tracked
by the test suite. It can be used in the following way:

```python
# -*- coding: utf-8 -*-

import system_tests


class AnInformativeName(metaclass=system_tests.CaseMeta):

    filename = "invalid_input_file"
    commands = ["$binary -c $import_file -i $filename"]
    retval = ["$abort_exit_value"]
    stdout = ["Reading $filename"]
    stderr = ["""A huge amount of error messages would be here that we absolutely do not care about. Actually everything in this string gets ignored, so we can just leave it empty.
"""
    ]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
```


[TOC](#TOC)

<div id="running-all-commands-under-valgrind"/>
### Running all commands under valgrind

The test suite can run all commands under a memory checker like
[valgrind](http://valgrind.org/) or [dr. memory](http://drmemory.org/). This
option can be enabled by adding the entry `memcheck` in the `General` section of
the configuration file, which specifies the command to invoke the memory
checking tool. The test suite will then prefix **all** commands with the
specified command.

For example this configuration file:

```ini
[General]
timeout: 0.1
memcheck: valgrind --quiet
```

will result in every command specified in the test cases being run as `valgrind
--quiet $command`.

When running your test cases under a memory checker, please take the following
into account:

- valgrind and dr. memory slow the program execution down by a factor of
  10-20. Therefore the test suite will increase the timeout value by a factor of
  20 or by the value specified in the option `memcheck_timeout_penalty` in the
  `General` section.

- valgrind reports by default on success to stderr, be sure to run it with
  `--quiet`. Otherwise successful tests will fail under valgrind, as unexpected
  output is present on stderr

- valgrind and ASAN cannot be used together

- Although the option is called `memcheck`, it can be used to execute all
  commands via a wrapper that has a completely different purpose (e.g. to
  collect test coverage).


[TOC](#TOC)

<div id="manually-expanding-variables"/>
### Manually expanding variables in strings

In case completely custom checks have to be run but one still wants to access
the variables from the test suite, the class `system_test.Case` provides the
function `expand_variables(self, string)`. It performs the previously described
variable substitution using the test suite's configuration file.

Unfortunately, it has to run in a class member function. The `setUp()` function
can be used for this, as it is run before each test. For example like this:

```python
class SomeName(metaclass=system_tests.CaseMeta):

	def setUp(self):
		self.commands = [self.expand_variables("$some_var/foo.txt")]
		self.stderr = [""]
		self.stdout = [self.expand_variables("$success_message")]
		self.retval = [0]
```

This example will work, as the test runner reads the data for `commands`,
`stderr`, `stdout` and `retval` from the class instance. What however will not
work is creating a new member in `setUp()` and trying to use it as a variable
for expansion, like this:

```python
class SomeName(metaclass=system_tests.CaseMeta):

	def setUp(self):
		self.new_var = "foo"
		self.another_string = self.expand_variables("$new_var")
```

This example fails in `self.expand_variables` because the expansion uses only
static class members (which `new_var` is not). Also, if you modify a static
class member in `setUp()` the changed version will **not** be used for variable
expansion, as the variables are saved in a new dictionary **before** `setUp()`
runs. Thus this:

```python
class SomeName(metaclass=system_tests.CaseMeta):

	new_var = "foo"

	def setUp(self):
		self.new_var = "bar"
		self.another_string = self.expand_variables("$new_var")
```

will result in `another_string` being "foo" and not "bar".


[TOC](#TOC)

<div id="hooks"/>
### Hooks

The `Case` class provides two hooks that are run after each command and after
all commands, respectively. The hook which is run after each successful command
has the following signature:

```python
post_command_hook(self, i, command)
```
with the following parameters:
- `i`: index of the command in the `commands` list
- `command`: a string of the actually invoked command

The hook which is run after all test takes no parameters except `self`:

```python
post_tests_hook(self)
```

By default, these hooks do nothing. They can be used to implement custom checks
after certain commands, e.g. to check if a file was created. Such a test can be
implemented as follows:

```python
# -*- coding: utf-8 -*-

import system_tests


class AnInformativeName(metaclass=system_tests.CaseMeta):

    filename = "input_file"
    output = "out"
    commands = ["$binary -o output -i $filename"]
    retval = [0]
    stdout = [""]
    stderr = [""]

    output_contents = """Hello World!
"""

    def post_tests_hook(self):
        with open(self.output, "r") as out:
            self.assertMultiLineEqual(self.output_contents, out.read(-1))
```


### Possible pitfalls

- Do not provide a custom `setUpClass()` function for the test
  cases. `setUpClass()` is used by `system_tests.Case` to store the variables
  for expansion.


[TOC](#TOC)

<div id="bash-test-cases"/>
## Bash test cases

- Previously, Exiv2 had some bash test scripts, which were saved as the file `EXIV2_DIR/test/*.sh`. We're going to rewrite them as Python test scripts and save them to the directory `EXIV2_DIR/tests/bash_tests`.
- These Python test scripts are based on [unittest](https://docs.python.org/3/library/unittest.html) and written in a common format, which is different from the format described in [Writing new tests](#writing-new-tests), but can be executed compatibly by `python3 runner.py`.

[TOC](#TOC)

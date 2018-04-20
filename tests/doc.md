# TL;DR

If you just want to write a simple test case, check out the file
`writing_tests.md`.

# Introduction

This test suite is intended for system tests, i.e. for running a binary with
certain parameters and comparing the output against an expected value. This is
especially useful for a regression test suite, but can be also used for testing
of new features where unit testing is not feasible, e.g. to test new command
line parameters.

The test suite is configured via `INI` style files using Python's builtin
[ConfigParser](https://docs.python.org/3/library/configparser.html)
module. Such a configuration file looks roughly like this:
``` ini
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

``` ini
some_var:     some value with whitespaces before and after    
```
is equivalent to this:
``` ini
some_var:some value with whitespaces before and after
```

The test suite itself uses the builtin `unittest` module of Python to discover
and run the individual test cases. The test cases themselves are implemented in
Python source files, but the required Python knowledge is minimal.

## Test suite

The test suite is configured via one configuration file whose location
automatically sets the root directory of the test suite. The `unittest` module
then recursively searches all sub-directories with a `__init__.py` file for
files of the form `test_*.py`, which it automatically interprets as test cases
(more about these in the next section). Python will automatically interpret each
directory as a module and use this to format the output, e.g. the test case
`regression/crashes/test_bug_15.py` will be interpreted as the module
`regression.crashes.test_bug_15`. Thus one can use the directory structure to
group test cases.

The test suite's configuration file should have the following form:

``` ini
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

``` ini
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


## Test cases

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

``` python
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
substitution of values is performed using the template module from Python's
string library via `safe_substitute`.

In the above example the command would thus expand to:
``` shell
/path/to/the/dir/build/bin/binary -c /path/to/the/dir/conf/main.cfg -i invalid_input_file
```
and similarly for `stdout` and `stderr`.

Once the substitution is performed, each command is run using Python's
`subprocess` module, its output is compared to the values in `stdout` and
`stderr` and its return value to `retval`. Please note that for portability
reasons the subprocess module is run with `shell=False`, thus shell expansions
or pipes will not work.

As the test cases are implemented in Python, one can take full advantage of
Python for the construction of the necessary lists. For example when 10 commands
should be run and all return 0, one can write `retval = 10 * [0]` instead of
writing 0 ten times. The same is of course possible for strings.

There are however some peculiarities with multiline strings in Python. Normal
strings start and end with a single `"` but multiline strings start with three
`"`. Also, while the variable names must be indented, new lines in multiline
strings must not or additional whitespaces will be added. E.g.:

``` python
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


## Advanced test cases

This section describes more advanced features that are probably not necessary
the "standard" usage of the test suite.


### Creating file copies

For tests that modify their input file it is useful to run these with a
disposable copy of the input file and not with the original. For this purpose
the test suite features a decorator which creates a copy of the supplied files
and deletes the copies after the test ran.

Example:
``` python
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


### Customizing the output check

Some tests do not require a "brute-force" comparison of the whole output of a
program but only a very simple check (e.g. that a string is present). For these
cases, one can customize how stdout and stderr checked for errors.

The `system_tests.Case` class has two public functions for the check of stdout &
stderr: `compare_stdout` & `compare_stderr`. They have the following interface:
``` python
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
``` python
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


### Manually expanding variables in strings

In case completely custom checks have to be run but one still wants to access
the variables from the test suite, the class `system_test.Case` provides the
function `expand_variables(self, string)`. It performs the previously described
variable substitution using the test suite's configuration file.

Unfortunately, it has to run in a class member function. The `setUp()` function
can be used for this, as it is run before each test. For example like this:
``` python
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
``` python
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
``` python
class SomeName(metaclass=system_tests.CaseMeta):

	new_var = "foo"

	def setUp(self):
		self.new_var = "bar"
		self.another_string = self.expand_variables("$new_var")
```

will result in `another_string` being "foo" and not "bar".


### Possible pitfalls

- Do not provide a custom `setUpClass()` function for the test
  cases. `setUpClass()` is used by `system_tests.Case` to store the variables
  for expansion.


## Running the test suite

The test suite is written for Python 3 and is not compatible with Python 2, thus
it must be run with `python3` and not with `python` (which is usually an alias
for Python 2).

Then navigate to the `tests/` subdirectory and run:
``` shell
python3 runner.py
```

One can supply the script with a directory where the suite should look for the
tests (it will search the directory recursively). If omitted, the runner will
look in the directory where the configuration file is located.

The runner script also supports the optional arguments `--config_file` which
allows to provide a different test suite configuration file than the default
`suite.conf`. It also forwards the verbosity setting via the `-v`/`--verbose`
flags to Python's unittest module.

Optionally one can provide the `--debug` flag which will instruct test suite to
print all command invocations and all expected and obtained outputs to the
standard output.

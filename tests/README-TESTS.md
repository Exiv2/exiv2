| Travis        | AppVeyor      | GitLab| Codecov| Repology| Chat |
|:-------------:|:-------------:|:-----:|:------:|:-------:|:----:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=0.27-maintenance)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/0.27-maintenance?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/0.27-maintenance) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/0.27-maintenance/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/0.27-maintenance) | [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/0.27-maintenance/graph/badge.svg)](https://codecov.io/gh/Exiv2/exiv2) | [![Packaging status](https://repology.org/badge/tiny-repos/exiv2.svg)](https://repology.org/metapackage/exiv2/versions) | [![#exiv2-chat on matrix.org](matrix-standard-vector-logo-xs.png)](https://matrix.to/#/#exiv2-chat:matrix.org) |
![Exiv2](../exiv2.png)

[README-TESTS](#README-TESTS)
The Exiv2 Test Suite is designed to run an executable with different inputs and compare
the output to an expected value. This is especially useful for a regression test
suite and for tests which are too complex for Unit Tests.

Exiv2 also has Unit Tests to test low level features of the code.
The unit tests are stored in unitTests/ and are not discussed in this document.

<div id="TOC"/>

1. [Running the test suite](#1)  
    1.1 [Writing new tests](#writing-new-tests)  
    1.2 [Tests derived from unittest](#tests-derived-from-unittest)  
    1.3 [Tests derived from system_tests](#tests-derived-from-system-tests)  
    1.4 [Test Suite Configuration](#test-suite-config)  
    &nbsp;&nbsp;&nbsp; - [Syntax](#ini-style)  
    &nbsp;&nbsp;&nbsp; - [Parameters](#parameters)  
    &nbsp;&nbsp;&nbsp; - [Multiline strings](#multiline-strings)  
    &nbsp;&nbsp;&nbsp; - [Paths and Variables](#paths)  
    1.5 [Test Scripts](#test-scripts)  
    1.6 [Advanced test scripts](#advanced-test-scripts)  
    &nbsp;&nbsp;&nbsp; - [Providing standard input to commands](#providing-standard-input-to-commands)  
    &nbsp;&nbsp;&nbsp; - [Using a different output encoding](#using-a-different-output-encoding)  
    &nbsp;&nbsp;&nbsp; - [Working with binary output](#working-with-binary-output)  
    &nbsp;&nbsp;&nbsp; - [Setting and modifying environment variables](#setting-and-modifying-environment-variables)  
    &nbsp;&nbsp;&nbsp; - [Creating file copies](#creating-file-copies)  
    &nbsp;&nbsp;&nbsp; - [Customizing the output check](#customizing-the-output-check)  
    &nbsp;&nbsp;&nbsp; - [Running all commands under valgrind](#running-all-commands-under-valgrind)  
    &nbsp;&nbsp;&nbsp; - [Manually expanding variables in strings](#manually-expanding-variables)  
    &nbsp;&nbsp;&nbsp; - [Hooks](#hooks)  
    &nbsp;&nbsp;&nbsp; - [Possible pitfalls](#possible-pitfalls)  
2. [bash tests](#bash-tests)  
    2.1 [Bash Reference Output](#reference-output)  
    2.2 [Bash Utilities in Python](#python-bash-utilities)  


<div id="1"/>
## 1. Running the test suite

The test suite is written for python 3.  All tests scripts are written in python.  To install and run the test suite:

```bash
$ cd tests
$ python3 -m pip install -r requirements.txt  # this only need to be done once
$ python3 runner.py
```
One can supply the script with a directory where the suite should look for the
tests (it will search the directory recursively). If omitted, the runner will
look in the directory where the configuration file is located. It is also
possible to pass a file as the parameter, the test suite will then only
run the tests using that file.

The runner script also supports the optional arguments `--config_file` which
allows to provide a different test suite configuration file than the default
`suite.conf`. It also forwards the verbosity setting via the `-v`/`--verbose`
flags to python's unittest module.

The optional runner.py argument `--debug` instruct the test suite to
print all command invocations and all expected and obtained outputs to the
standard output.

[TOC](#TOC)
<div id="writing-new-tests"/>
### 1.1 Writing new tests

The test suite is intended to run an executable and compare its standard output,
standard error and return value against provided values. This is implemented
using python's [unittest](https://docs.python.org/3/library/unittest.html) module.

To create new tests, proceed as follows:

1. Choose an appropriate subdirectory where the test belongs. If none fits
   create a new one and put an empty `__init__.py` file there.
2. Create a new file with a name to match test_*.py and write your test script.
3. Run the test suite:

```bash
python3 runner.py               # automatically find and run test scripts
					[test.py]   # run one test script
					-v          # verbose output
```

[TOC](#TOC)
<div id="tests-derived-from-unittest"/>
### Tests derived from unittest

You can write standard [unittest](https://docs.python.org/3/library/unittest.html) test cases.
The keys steps are :
1. Do something.\
    For example, execute some shell commands through the `BT.Exec` class.
2. Check if things are OK.\
    Any exception you raise will cause the test case to fail.

For example:

```py
# -*- coding: utf-8 -*-
import os
import unittest
from system_tests import BT                 # import system_tests, which has been loaded into sys.path
class TestCases(unittest.TestCase):
    def setUp(self):
        """ This function is executed before each test case. """
        os.makedirs(BT.Config.tmp_dir, exist_ok=True)
        os.chdir(BT.Config.tmp_dir)         # switch to the temporary directory to test

    def simple_test(self):                  # define a test function
        e = BT.Exec('exiv2 --version')  # execute a command in the shell
        assert e.returncode == 0:
        if 'exiv2 0.27.4.9' not in e.stdout:
            raise RuntimeError('Wrong version')

    def addmoddel_test(self):               # define another test function
        jpg      = 'exiv2-empty.jpg'
        BT.copyTestFile(jpg)
        out      = BT.Output()
        out     += BT.Exec('addmoddel {jpg}', vars())
        out     += BT.Exec('exiv2 -pv {jpg}', vars())
        BT.reportTest('addmoddel', out)
```

system\_tests.BT is defined in tests/utils/ and provides functions and classes that are compatible with different platforms, making it easier to write test scripts. You can find detailed documentation in `tests/utils/__init__.py`.

[TOC](#TOC)
<div id="tests-derived-from-system-tests"/>
### Tests derived from system_tests

In addition to unittest test scripts, you can also write a declarative
test script by inheriting `system_tests.CaseMeta`. For example:

```python
# -*- coding: utf-8 -*-
import system_tests
class UniqueTestName(metaclass=system_tests.CaseMeta):
    filename = "$data_path/test_file"
    commands = ["$exiv2 $filename", "$exiv2 $filename" + '_2']
    stdout   = [""] * 2
    stderr   = ["""$exiv2_exception_msg $filename:
$kerFailedToReadImageData
"""] * 2
    retval   = [1] * 2
```

The test suite will run the provided each command and compare the output, stderr and retval
with the values you provide.

The strings such as **$variable** are variables which may be defined in this test's class
or the suite configuration file.

[TOC](#TOC)
<div id="test-suite-config"/>
## Test suite configuration

The test suite uses the module `unittest` to discover
and run test scripts which are written in python.

The test suite is configured using a configuration file whose location
automatically becomes the root directory of the test suite. The unittest module
then recursively searches all sub-directories with a `__init__.py` file for
files of the form `test_*.py`, which it automatically interprets as test cases
(more about these in the next section). Python will automatically interpret each
directory as a module and use this to format the output, e.g. the test case
`regression/crashes/test_bug_15.py` will be interpreted as the module
`regression.crashes.test_bug_15`. The directory structure is used to group test cases.


<div id="ini-style"/>
#### Configuration file syntax

The test suite is configured using ini style files which are parsed 
[ConfigParser](https://docs.python.org/3/library/configparser.html)
module. Here is a typical ini file:

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
documentation.  ConfigParser module is used with the following defaults:
- Comments are started by `#` only
- The separator between a variable and the value is `:`
- Multiline comments can have empty lines
- Extended Interpolation is used (this allows to refer to other sections when
  inserting values using the `${section:variable}` syntax)

Please keep in mind that leading and trailing whitespaces are **stripped** from
strings when extracting variable values.

[TOC](#TOC)
<div id="parameters"/>
#### Configuration file example

The test suite's configuration file has the following form:

```ini
[General]
timeout: 0.1

[paths]
program: ../build/bin/program
exiv2:   ../build/bin/exiv2
geotag:  ../bin/bin/geotag

important_file: ../conf/main.cfg

[variables]
abort_error: ERROR
abort_exit value: 1
```

The **[General]** section only contains the optional timeout parameter (default 1.0).
The timeout is the maximum time in seconds for any command that to run before being aborted.
Commands which loop or loop or hang will be aborted to enable for test driven development.

The **[variables]** and **[paths]** sections define global values that are accessible to every
test script. Section **[variables]** enable you to define common output strings.
For example an error message or value can be conveniently declared in the configuration
and referenced by many test scripts. Section **[paths]** provides variables for scripts
to know absolute paths without knowledge of the environment.  The path variables are
interpreted relative to the test suite root and expanded to absolute paths for reference
by test scripts. 

The variables in both section **[paths]** and **[variables]** are merged
before being accessed by the test script.  An error will be
reported if local or **[path]** or **[variables]** share the same name

However, sometimes environment variables are convenient to implement variable
paths or platform differences such as different build directories or file
extensions. For this, the test suite supports the `ENV` and `ENV fallback`
sections. In conjunction with the extended interpolation of the ConfigParser
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
exiv2: ${ENV:variable_prefix}/bin/exiv2${ENV:file_extension}
geotag: ${ENV:variable_prefix}/bin/geotag${ENV:file_extension}
important_file: ../conf/main.cfg

[variables]
abort_error: ERROR
abort_exit value: 1
```

The `ENV` section is similar to the paths section insofar as the
variables are extracted from the environment with the given name. E.g. the
variable file\_extension would be set to the value of the environment variable
FILE\_EXT. If the environment variable is not defined, then the test suite will
look in the **[ENV fallback]** section. E.g. in the above example
variable\_prefix has the fallback or default value of `../build` which will be
used if the environment variable `PREFIX` is not set. If no fallback is provided
then an empty string is used instead, which would happen to file\_extension if
FILE\_EXT is not defined.

This can be combined with the extended interpolation of python's ConfigParser,
which allows to include variables from arbitrary sections into other variables
using the ${sect:var\_name} syntax. This would be expanded to the value of
var\_name from the section sect. The above example only utilizes this in the
**[paths]** section, however it can also be used in the **[variables]** section.

Returning to the example config file, the path executabl would be inferred in the
following steps:

1. extract PREFIX & FILE_EXT from the environment, if they don't exist use
   the default values from ENV fallback or ""
2. substitute the strings ${ENV:variable_prefix} and ${ENV:file_extension}
3. expand the relative path to an absolute path

Please note: While ini files supports variable names with whitespaces and/or '-',
these names are invalid in python and must be avoided.

[TOC](#TOC)
<div id="test-scripts"/>
### Test Scripts

Test scripts are written in python and use the unittest module. The filename
of a script is required to start with `test_` and have the file extension `py`.

A test script should test one logical unit, e.g. test for regressions of a certain
bug or check if a command line option works. Each test case can run multiple
commands which results are compared to an expected standard output, standard
error and return value. Should differences arise or should one of the commands
take too long, then an error message with the exact differences is shown to the
user.

An typical test script is:

```python
# -*- coding: utf-8 -*-
import system_tests
class AnInformativeName(metaclass=system_tests.CaseMeta):
    filename = "invalid_input_file"
    commands = ["$binary -c $import_file -i $filename"]
    retval   = ["$abort_exit_value"]
    stdout   = ["Reading $filename"]
    stderr   = [ """$abort_error
error in $filename
"""]
```

The meta-class `system_tests.CaseMeta` performs the necessary setup to
run the test. When adding new tests choose a unique class name
that briefly summarizes the test. Note that the file name (without the
extension) with the directory structure is interpreted as the module by python
and pre-pended to the class name when reporting about the tests. E.g. the file
`regression/crashes/test_bug_15.py` with the class `OutOfBoundsRead` gets
reported as `regression.crashes.test_bug_15.OutOfBoundsRead` already including
a brief summary of this test.

You must define the arrays commands, retval, stdout and stderr.  The test suite
substitutes all $variable values such as $filename and $abort error.
In this case $filename is defined locally and $abort_error is defined globally from suite.conf.
All variables are treated as python strings.

The substitution of values is performed using safe_substitute in the python string module.

After substitution each command is run using the python subprocess module.
The stdout, stderr and retval values from executing the command are compared to
the reference in the test script.

Please be away that for portability reasons the subprocess module is run
with `shell=False`.  Shell expansions, pipes and redirections are not available.

As test scripts are written python, you can use python fall eatures.  For example
when 10 commands should be run and all expected to return 0 with no output:

```python
   retval = [0] * len(commands)
   stdout = [''] * len(commands)
   errout = [''] * len(commands)
```
Which is simpler than:

```python
   retval = [0,0,0,0,0,0,0,0,0,0] 
   stdout = ['','','','','','','','','',''] 
   errout = ['','','','','','','','','','']
```

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
    stderr = ["""something
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
    stdin  = [ "read file a",
                 None
    ]
    stdout = [ "Reading...",
               ""
    ]
    stderr = [ "Error",
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

[TOC](#TOC)
<div id="setting-and-modifying-environment-variables"/>
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
    output   = "out"
    commands = ["$binary -o output -i $filename"]
    retval   = [0]
    stdout   = [""]
    stderr   = [""]

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
<div id="bash-tests"/>
## bash tests

- Prior to Exiv2 v0.27.4, exiv2 had bash test scripts in the /test/ directory.
- With Exiv2 v0.27.4 the bash scripts have been rewritten in python.
- The new pythonic bash\_tests are invoked by runner.py and stored in tests/bash_tests/testcases.py

The bash test were implemented in test/*.sh as follows:

```bash
$ ls test/*.sh
test/addmoddel.sh	    test/exiv2-test.sh	test/iptctest.sh	test/png-test.sh	    test/tiff-test.sh	test/write2-test.sh
test/bash_tests.sh	    test/geotag-test.sh	test/iso65k-test.sh	test/preview-test.sh	test/unit_test.sh	test/xmpparser-test.sh
test/conversions.sh	    test/icc-test.sh	test/modify-test.sh	test/stdin-test.sh	    test/version_test.sh
test/crw-test.sh	    test/imagetest.sh	test/nls-test.sh	test/stringto-test.sh	test/webp-test.sh
test/exifdata-test.sh	test/iotest.sh		test/path-test.sh	test/tests.sh	    	test/write-test.sh
$ 
```

The pythonic implementation are stored in tests/bash_tests/testcases.py and there is python function for each test:

```bash
$ grep '^    def ' tests/bash_tests/testcases.py 
    def setUp(self):
    def tearDown(self):
    def addmoddel_test(self):
    def conversions_test(self):
    def crw_test(self):
    def exifdata_test(self):
    def exiv2_test(self):
    def geotag_test(self):
    def icc_test(self):
    def image_test(self):
    def io_test(self):
    def iptc_test(self):
    def iso65k_test(self):
    def modify_test(self):
    def nls_test(self):
    def path_test(self):
    def png_test(self):
    def preview_test(self):
    def stdin_test(self):
    def stringto_test(self):
    def tiff_test(self):
    def version_test(self):
    def webp_test(self):
    def write_test(self):
    def write2_test(self):
    def xmpparser_test(self):
$
```

As you can see, with the exception of `setUp()` and `tearDown()`, the function names are the same as their bash ancestors.

The code in the individual tests is very similar.  For example geotag_test in python is:

```python
import os
import re
import unittest
from system_tests import BT

    def geotag_test(self):
        # Test driver for geotag
        jpg      = 'FurnaceCreekInn.jpg'
        gpx      = 'FurnaceCreekInn.gpx'
        for i in [jpg, gpx]:
            BT.copyTestFile(i)

        out      = BT.Output()
        out     += '--- show GPSInfo tags ---'
        out     += BT.Exec('exiv2 -pa --grep GPSInfo    {jpg}', vars())

        out     += '--- deleting the GPSInfo tags'
        for tag in BT.Exec('exiv2 -Pk --grep GPSInfo    {jpg}', vars()).stdout.split('\n'):
            tag  = tag.rstrip(' ')
            out += BT.Exec('exiv2 -M"del {tag}"         {jpg}', vars())
        out     += BT.Exec('exiv2 -pa --grep GPS        {jpg}', vars(), assert_returncode=[0, 1])

        out     += '--- run geotag ---'
        e        = BT.Exec('geotag -ascii -tz -8:00     {jpg} {gpx}', vars())
        out     += ' '.join(e.stdout.split('\n')[0].split(' ')[1:])

        out     += '--- show GPSInfo tags ---'
        out     += BT.Exec('exiv2 -pa --grep GPSInfo    {jpg}', vars())

        BT.reportTest('geotag-test', out)
```

geotag_tests.sh is as follows:

```bash
#!/usr/bin/env bash
# Test driver for geotag

source ./functions.source

(   jpg=FurnaceCreekInn.jpg
    gpx=FurnaceCreekInn.gpx
    copyTestFiles $jpg $gpx

    echo --- show GPSInfo tags ---
    runTest                      exiv2 -pa --grep GPSInfo $jpg
    tags=$(runTest               exiv2 -Pk --grep GPSInfo $jpg  | tr -d '\r') # MSVC puts out cr-lf lines
    echo --- deleting the GPSInfo tags
    for tag in $tags; do runTest exiv2 -M"del $tag" $jpg; done
    runTest                      exiv2 -pa --grep GPS     $jpg
    echo --- run geotag ---
    runTest                      geotag -ascii -tz -8:00 $jpg $gpx | cut -d' ' -f 2-
    echo --- show GPSInfo tags ---
    runTest                      exiv2 -pa --grep GPSInfo $jpg
) > $results 2>&1

reportTest
```

[TOC](#TOC)
<div id="reference-output"/>
### Bash Reference Output

The reference output for bash\_tests is stored in test/data/test\_name.out.  When bash\_test execute, they output to the directory test/temp.  With system\_tests, the reference output is stored in the test script itself.

[TOC](#TOC)
<div id="python-bash-utilities"/>
### Bash Utilities in Python

In the bash tests, many system utilities such as diff, grep, cut and sed are using to transform the "raw" output of the commands into reference output.   The module system\_tests.BT provides function to emulate those utilities.

Here's a session to introduce using system_tests from the python shell:

```
613 rmills@rmillsmm-local:~/gnu/github/exiv2/fix_issue_1485/tests $ python3
Python 3.8.2 (default, Sep 24 2020, 19:37:08) 
[Clang 12.0.0 (clang-1200.0.32.21)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import system_tests
>>> x=system_tests.BT.verbose_version(True)
exiv2                0.27.4
platform             apple
compiler             Clang
bits                 64
dll                  1
debug                1
cplusplus            199711
version              Apple LLVM 12.0.0 (clang-1200.0.32.21)
date                 Mar 16 2021
time                 07:29:25
processpath          /Users/rmills/gnu/github/exiv2/fix_issue_1485/build/bin
package_name         exiv2
curl                 0
executable           /Users/rmills/gnu/github/exiv2/fix_issue_1485/build/bin/exiv2
library              [ /Users/rmills/gnu/github/exiv2/fix_issue_1485/build/lib/libexiv2.0.27.4.9.dylib   +41 ]
have_strerror_r      [ 1   +0 ]
...
>>> help(system_tests.BT)
Help on package utils:

NAME
    utils

DESCRIPTION
    This package provides code that can be invoked by all test cases, which are stored in these scripts:
    - config.py   : Contains configuration parameters for the test suite.
    - common.py   : Contains some code that is decoupled from the test cases, even from the Exiv2 project.
    - exec.py     : Contains the Exec class for executing shell commands.
    - tests.py    : Contains some code that is coupled to the test cases, and can only be called by a few test cases.
    
    You can find many examples in tests/bash_tests/testcases.py .

PACKAGE CONTENTS
    common
    config
    exec
    tests

DATA
    log = <utils.common.Log object>

FILE
    /Users/rmills/gnu/github/exiv2/fix_issue_1485/tests/utils/__init__.py
```

[TOC](#TOC)



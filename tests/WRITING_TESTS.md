## Writing new tests

The test suite is intended to run a binary and compare its standard output,
standard error and return value against provided values. This is implemented
using Python's `unittest` module and thus all test files are Python files.

The simplest test has the following structure:
``` python
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

When creating new tests, follow roughly these steps:

1. Choose an appropriate subdirectory where the test belongs. If none fits
   create a new one and put an empty `__init__.py` file there.

2. Create a new file with a name matching `test_*.py`. Copy the class definition
   from the above example and choose an appropriate class name.

3. Run the test suite via `python3 runner.py` and ensure that your test case is
   actually run! Either run the suite with the `-v` option which will output all
   test cases that were run or simply add an error and check if errors occur.

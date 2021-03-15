"""
This package provides code that can be invoked by all test cases, which are stored in these scripts:
- config.py   : Contains configuration parameters for the test suite.
- common.py   : Contains some code that is decoupled from the test cases, even from the Exiv2 project.
- exec.py     : Contains the Exec class for executing shell commands.
- tests.py    : Contains some code that is coupled to the test cases, and can only be called by a few test cases.

You can find many examples in tests/bash_tests/testcases.py .
"""

from .config    import *
from .common    import *
from .exec      import *
from .tests     import *


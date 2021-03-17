# -*- coding: utf-8 -*-

import system_tests
import os
import unittest

bSkip=not os.path.isfile(system_tests.unit_tests) and not os.path.isfile(system_tests.unit_tests + '.exe')
if bSkip:
    raise unittest.SkipTest('*** %s does not exist ***' % system_tests.unit_tests)

system_tests.BT.runTest(system_tests.unit_tests,raw=True)

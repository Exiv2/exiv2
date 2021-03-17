# -*- coding: utf-8 -*-

import system_tests
import os
import unittest

bSkip=not (os.path.isfile(system_tests.unit_tests) or os.path.isfile(system_tests.unit_tests + '.exe'))
if bSkip:
    msg='*** %s does not exist ***' % system_tests.unit_tests
    print(msg)
    raise unittest.SkipTest(msg)

system_tests.BT.runTest(system_tests.unit_tests,raw=True)
print('')

# -*- coding: utf-8 -*-

import system_tests
import os
import unittest

bSkip=not os.path.isfile(system_tests.unit_tests)
if bSkip:
    raise unittest.SkipTest('*** %s does not exist ***' % system_tests.unit_tests)

lines=system_tests.BT.runTest(system_tests.unit_tests)
for line in lines:
    print(line)

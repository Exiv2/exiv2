# -*- coding: utf-8 -*-

import system_tests
import unittest

# test needs system_tests.BT.vv['enable_bmff']=1
bSkip=system_tests.BT.verbose_version().get('enable_bmff')!='1'
if bSkip:
    raise unittest.SkipTest('*** requires enable_bmff=1 ***')

class BmffImageReadMetadataOutOfBoundsRead(metaclass=system_tests.CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1570
    """
    url = "https://github.com/Exiv2/exiv2/issues/1570"
    filename = "$data_path/issue_1570_poc.bmff"

    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2 $filename"]
        stdout = [""]
        stderr = [
            """Exiv2 exception in print action for file $filename:
$kerCorruptedMetadata
"""]
        retval = [1]

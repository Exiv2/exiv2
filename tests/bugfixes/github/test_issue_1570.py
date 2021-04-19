# -*- coding: utf-8 -*-

<<<<<<< HEAD
import system_tests
import unittest

# test needs system_tests.BT.vv['enable_bmff']=1
bSkip=system_tests.BT.verbose_version().get('enable_bmff')!='1'
if bSkip:
    raise unittest.SkipTest('*** requires enable_bmff=1 ***')

class BmffImageReadMetadataOutOfBoundsRead(metaclass=system_tests.CaseMeta):
=======
from system_tests import CaseMeta, path


class BmffImageReadMetadataOutOfBoundsRead(metaclass=CaseMeta):
>>>>>>> 386e464e1... Regression test for https://github.com/Exiv2/exiv2/issues/1570.
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1570
    """
    url = "https://github.com/Exiv2/exiv2/issues/1570"
<<<<<<< HEAD
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
=======

    filename = path("$data_path/issue_1570_poc.bmff")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
"""$filename1: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
>>>>>>> 386e464e1... Regression test for https://github.com/Exiv2/exiv2/issues/1570.

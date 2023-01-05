# -*- coding: utf-8 -*-

from system_tests import CaseMeta, FileDecoratorBase, path
from struct import *

# The PoC is a fairly large file, mostly consisting of zero bytes,
# so it would be a waste of storage to check it into the repo.
# Instead, we can generate the PoC with a small amount of code:
class CreatePoC(FileDecoratorBase):
    """
    This class copies files from test/data to test/tmp
    Copied files are NOT removed in tearDown
    Example: @CopyTmpFiles("$data_path/test_issue_1180.exv")
    """

    #: override the name of the file list
    FILE_LIST_NAME = '_tmp_files'

    def setUp_file_action(self, expanded_file_name):
        size = 0x20040
        contents = pack('<2sI8sHHIIHHII', bytes(b'II'), 14, bytes(b'HEAPCCDR'), \
                        1, 0x300b, size - 26, 12, 1, 0x102a, size - 38, 12) + \
                        bytes(bytearray(size-38))
        f = open(expanded_file_name, 'wb')
        f.write(contents)
        f.close()

    def tearDown_file_action(self, f):
        """
        Do nothing.   We don't clean up TmpFiles
        """

# This decorator generates the PoC file.
@CreatePoC("$tmp_path/issue_ghsa_hqjh_hpv8_8r9p_poc.crw")

class CrwMapDecodeArrayInfiniteLoop(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-hqjh-hpv8-8r9p
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-hqjh-hpv8-8r9p"

    filename = path("$tmp_path/issue_ghsa_hqjh_hpv8_8r9p_poc.crw")

    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
"""Exiv2 exception in print action for file $filename:
$kerCorruptedMetadata
"""]
    retval = [1]

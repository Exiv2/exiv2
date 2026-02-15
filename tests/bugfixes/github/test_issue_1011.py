# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Test_issue_1011(metaclass=CaseMeta):

    filename = path("$data_path/Jp2Image_readMetadata_loop.poc")
    commands = ["$exiv2 " + filename]
    stdout   = [""]
    stderr   = ["""$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    retval = [1]
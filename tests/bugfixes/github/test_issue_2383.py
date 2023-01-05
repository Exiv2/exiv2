# -*- coding: utf-8 -*-

from system_tests import CaseMeta, check_no_ASAN_UBSAN_errors

class issue_2383_QuickTimeVideo_userDataDecoder_null_deref(metaclass=CaseMeta):
    url      = "https://github.com/Exiv2/exiv2/issues/2383"
    filename = "$data_path/issue_2383_poc.mp4"
    commands = ["$exiv2 $filename"]
    retval   = [1]
    stderr   = ["""$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    stdout   = [""]

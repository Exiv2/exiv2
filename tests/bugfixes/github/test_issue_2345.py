# -*- coding: utf-8 -*-

from system_tests import CaseMeta, check_no_ASAN_UBSAN_errors

class issue_2345_QuickTimeVideo_tagDecoder_buffer_overflow(metaclass=CaseMeta):
    url      = "https://github.com/Exiv2/exiv2/issues/2345"
    filename = "$data_path/issue_2345_poc.mp4"
    commands = ["$exiv2 -q -pa $filename"]
    retval   = [1]
    stderr   = ["""$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    compare_stdout = check_no_ASAN_UBSAN_errors

# -*- coding: utf-8 -*-

from system_tests import CaseMeta, check_no_ASAN_UBSAN_errors

class issue_2427_BmffImage_brotliUncompress_memleak(metaclass=CaseMeta):
    url      = "https://github.com/Exiv2/exiv2/issues/2427"
    filename = "$data_path/issue_2427_poc.jpg"
    commands = ["$exiv2 $filename"]
    retval   = [1]
    stderr   = ["""$exiv2_exception_message $filename:
CL_SPACE
"""]
    stdout   = [""]

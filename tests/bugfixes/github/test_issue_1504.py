# -*- coding: utf-8 -*-

import system_tests
class issue_1504_metacopy(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1504"
    commands = ["$metacopy"
               ,"$metacopy -h"
               ]
    retval = [ 1,2] 
    stderr = [ """metacopy: Read and write files must be specified
""",""]
    stdout = ["""
Reads and writes raw metadata. Use -h option for help.
Usage: metacopy [-iecaph] readfile writefile
""","""
Reads and writes raw metadata. Use -h option for help.
Usage: metacopy [-iecaph] readfile writefile

Options:
   -i      Read Iptc data from readfile and write to writefile.
   -e      Read Exif data from readfile and write to writefile.
   -c      Read Jpeg comment from readfile and write to writefile.
   -x      Read XMP data from readfile and write to writefile.
   -a      Read all metadata from readfile and write to writefile.
   -p      Preserve existing metadata in writefile if not replaced.
   -h      Display this help and exit.

"""]

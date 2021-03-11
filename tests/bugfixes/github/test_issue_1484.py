# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/Stonehenge.exv")

class test_issue_1484(metaclass=CaseMeta):
    url       = "https://github.com/Exiv2/exiv2/issues/1484"
    filename  = path("$tmp_path/Stonehenge.exv")
    commands  = [ "$exiv2 -g Copyright                            $filename" 
                , "$exiv2 -M\"set Exif.Image.Copyright Ascii ''\" $filename"
                , "$exiv2 -g Copyright                            $filename"
                ]
    stdout   =  ["","","""Exif.Image.Copyright                         Ascii       1  
"""]
    stderr = [""]*len(commands)
    retval = [ 1,0,0]

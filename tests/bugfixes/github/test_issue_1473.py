# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/Stonehenge.exv")

class test_issue_1473(metaclass=CaseMeta):
    url       = "https://github.com/Exiv2/exiv2/issues/1473"
    filename  = path("$tmp_path/Stonehenge.exv")
    commands  = [ "$exiv2 -g shown/i                              $filename" 
                , "$exiv2 -M\"set Xmp.iptc.LocationShown Lost\" $filename"
                , "$exiv2 -g shown/i                              $filename"
                ]
    stdout   =  ["","","""Xmp.iptc.LocationShown                       XmpText     4  Lost
"""]
    stderr = [""]*len(commands)
    retval = [ 1,0,0]

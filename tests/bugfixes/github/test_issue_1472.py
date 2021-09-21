# -*- coding: utf-8 -*-
import os

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/test_issue_1472.jpg")

class test_issue_1472Test(metaclass=CaseMeta):

    filename  = path("$tmp_path/test_issue_1472.jpg")
    exvfile   = path("$tmp_path/test_issue_1472.exv")

    def setUp(self):
        if os.path.isfile(self.exvfile):
            os.remove(self.exvfile)

    commands  = [ "$exiv2 -pa --grep LensType2   $filename"
                , "$exiv2  ex                    $filename"
                , "$exiv2 -pa --grep LensType2   $exvfile"
                , "$exiv2  rm                    $filename"
                , "$exiv2 -pa --grep LensType2   $filename"
                , "$exiv2  in                    $filename"
                , "$exiv2 -pa --grep LensType2   $filename"
                ]
    stdout   =  ["Exif.Sony2010e.LensType2                     Short       1  1024\n"
                ,""
                ,"Exif.Sony2010e.LensType2                     Short       1  1024\n"
                ,""
                ,""
                ,""
                ,"Exif.Sony2010e.LensType2                     Short       1  1024\n"
                ]
    stderr = [""]*len(commands)
    retval = [ 0,0,0,0,1,0,0]

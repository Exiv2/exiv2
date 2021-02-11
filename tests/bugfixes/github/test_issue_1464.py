# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/test_issue_1464.exv")

class test_issue_1464Test(metaclass=CaseMeta):

    filename  = path("$tmp_path/test_issue_1464.exv")
    commands  = [  "$exiv2 -K Exif.Sony2010e.WB_RGBLevels              $filename"
                ,  "$exiv2 -M\"set Exif.Sony2010e.WB_RGBLevels 9 1 1\" $filename"
                ,  "$exiv2 -K Exif.Sony2010e.WB_RGBLevels              $filename"
                ]
    stdout   =  ["Exif.Sony2010e.WB_RGBLevels                  Short       3  598 256 442\n"
                ,""
                ,"Exif.Sony2010e.WB_RGBLevels                  Short       3  9 1 1\n"
                ]
    stderr = [""]*len(commands)
    retval = [ 0]*len(commands)

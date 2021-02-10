# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/test_issue_1464.exv")

class test_issue_1180Test(metaclass=CaseMeta):

    filename  = path("$tmp_path/test_issue_1464.exv")
    commands  = [  "$exiv2 -K Exif.Sony2010e.WB_RGBLevels       $filename"
                ]
    stdout   =  ["Exif.Image.DateTime                          Ascii      50  2020-04-24 17:44:15 \n"
                ,""
                ,"Exif.Image.DateTime                          Ascii      50  2020-04-24 17:44:15 \n"
                ,""
                ,"Exif.Image.DateTime                          Ascii      20  2020:04:24 15:44:15\n"
                ,""
                ,"Exif.Image.DateTime                          Ascii      20  2020:04:24 15:44:15\n"
                ]
    stderr = [""]*len(commands)
    retval = [ 0]*len(commands)

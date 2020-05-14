# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/test_issue_1180.exv")

class test_issue_1180Test(metaclass=CaseMeta):

    filename  = path("$tmp_path/test_issue_1180.exv")
    dash_t    = path("$tmp_path/20200424_174415.exv")      # -t renames file
    dash_T    = path("$tmp_path/20200424_154415.exv")      # -T renames file
    commands  = [  "$exiv2 -K Exif.Image.DateTime       $filename"
                ,  "$exiv2 -t          --force          $filename"
                ,  "$exiv2 -K Exif.Image.DateTime       $dash_t"
                ,  "$exiv2 -a -02:00   --force          $dash_t"
                ,  "$exiv2 -K Exif.Image.DateTime       $dash_t"
                ,  "$exiv2 -t          --force          $dash_t"
                ,  "$exiv2 -K Exif.Image.DateTime       $dash_T"
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

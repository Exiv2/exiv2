# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path

@CopyTmpFiles("$data_path/pr_2425_poc1.jpg")
class testExiv2AppExtractOverrideFile(metaclass=CaseMeta):
    url         = "https://github.com/Exiv2/exiv2/issues/2425"
    description = "Fix bug 1. reported in https://github.com/Exiv2/exiv2/issues/1934"
    
    filename_jpg = "$tmp_path/pr_2425_poc1.jpg"
    filename_exv = "$tmp_path/pr_2425_poc1.exv"
    
    commands = ["$exiv2 --force --extract XXeix $filename_jpg",
                "$exiv2 --print a $filename_exv",
                "$exiv2 --force --extract XXex $filename_jpg",
                "$exiv2 --print a $filename_exv"]
    
    retval = [0]*4
    stderr = [""]*4
    stdout = ["""""",
              """Exif.Image.ImageDescription                  Ascii      11  Test image
Iptc.Application2.Subject                    String     12  Test subject
Xmp.dc.subject                               XmpBag      1  Test subject
""",
              """""",
              """Exif.Image.ImageDescription                  Ascii      11  Test image
Xmp.dc.subject                               XmpBag      1  Test subject
"""]

# -*- coding: utf-8 -*-

import shutil
from system_tests import CaseMeta, path

class test_exifprint_lint(metaclass=CaseMeta):
    url       = "https://github.com/Exiv2/exiv2/pull/1738"
    # test_issue_1473 and test_issue_1484 from the bugfixes suite modify
    # tmp_path/Stonehenge.exv and that suite can run at the same time as this
    # one, so this test uses a file name of its own.
    original  = path("$data_path/Stonehenge.exv")
    filename  = path("$tmp_path/Stonehenge_exifprint_lint.exv")

    def setUp(self):
        shutil.copyfile(self.original, self.filename)

    commands  = [ "$exifprint --lint                                  $filename" 
                , '$exiv2 -M"set Exif.Image.ImageDescription Short 3" $filename'
                , "$exifprint --lint                                  $filename"
                ]
    stderr   =  ["""Exif.Nikon3.ExposureTuning type Undefined (7) expected Short (3)
""","","""Exif.Image.ImageDescription type Short (3) expected Ascii (2)
Exif.Nikon3.ExposureTuning type Undefined (7) expected Short (3)
"""
]
    stdout = [""]*len(commands)
    retval = [2,0,2]

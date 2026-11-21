# -*- coding: utf-8 -*-

import shutil

from system_tests import CaseMeta, path

class test_exifprint_lint(metaclass=CaseMeta):
    url       = "https://github.com/Exiv2/exiv2/pull/1738"
    original_file = path("$data_path/Stonehenge.exv")
    filename  = path("$tmp_path/Stonehenge_exifprint_lint.exv")
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

    def setUp(self):
        # The bugfix suite may be editing Stonehenge.exv in parallel.
        shutil.copyfile(self.original_file, self.filename)

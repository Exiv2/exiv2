# -*- coding: utf-8 -*-

import system_tests

class Nikon_ShutterMode_MechShutterCount(metaclass=system_tests.CaseMeta):
    url      = "https://github.com/Exiv2/exiv2/pull/1444"
    filename = "$data_path/CH0_0174.exv"
    commands = ["$exiv2 -g shutter/i $filename"]
    stderr   = [""]
    stdout   = ["""Exif.Nikon3.ShutterMode                      Short       1  Auto (Electronic Front Curtain)
Exif.Nikon3.MechanicalShutterCount           Long        1  174
Exif.Nikon3.ShutterCount                     Long        1  174
"""]
    retval = [0]

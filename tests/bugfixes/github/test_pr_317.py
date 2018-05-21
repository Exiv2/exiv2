# -*- coding: utf-8 -*-

import system_tests


class CanonEOSM100(metaclass=system_tests.CaseMeta):

    filename = "$data_path/exiv2-pr317.exv"
    commands = ["$exiv2 -pa --grep model/i $filename"]

    stdout = ["""Exif.Image.Model                             Ascii      15  Canon EOS M100
Exif.Canon.ModelID                           Long        1  EOS M100
Exif.Photo.LensModel                         Ascii      29  EF-M15-45mm f/3.5-6.3 IS STM
"""
	]
    stderr = [""]
    retval = [0] 

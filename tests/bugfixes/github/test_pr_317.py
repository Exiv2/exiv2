# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class CanonEOSM100(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr317.exv")
    commands = ["$exiv2 -pa --grep model/i $filename"]

    stdout = ["""Exif.Image.Model                             Ascii      15  Canon EOS M100
Exif.Canon.ModelID                           Long        1  EOS M100
Exif.Photo.LensModel                         Ascii      29  EF-M15-45mm f/3.5-6.3 IS STM
"""
    ]
    stderr = [""]
    retval = [0]

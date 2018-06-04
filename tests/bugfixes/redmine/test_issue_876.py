# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class Canon_EF_35mm_f_slash_2_IS_USM_Lens(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/876"

    filename = path("$data_path/exiv2-bug876.jpg")

    commands = ["$exiv2 -pt --grep Model $filename"]

    stdout = ["""Exif.Image.Model                             Ascii      13  Canon EOS 6D
Exif.Canon.ModelID                           Long        1  EOS 6D
Exif.Canon.LensModel                         Ascii      74  EF35mm f/2 IS USM
"""]
    stderr = [""]
    retval = [0]

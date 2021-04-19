# -*- coding: utf-8 -*-

import system_tests


class CrashAfterLightzoneEdit(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/479"

    filename = "$data_path/exiv2-bug479.jpg"

    commands = ["$exiv2 -u -b -pt $filename"]

    stdout = ["""Exif.Image.ImageWidth                        Short       1  3173
Exif.Image.ImageLength                       Short       1  2011
Exif.Image.Software                          Ascii      10  LightZone
Exif.Image.HostComputer                      Ascii      20  Linux 2.6.15-23-686
Exif.Image.ExposureTime                      Rational    1  3.2 s
Exif.Image.FNumber                           Rational    1  F9
Exif.Image.ExifTag                           Long        1  784
Exif.Photo.ExifVersion                       Undefined   4  2.20
Exif.Photo.PixelXDimension                   Short       1  3173
Exif.Photo.PixelYDimension                   Short       1  2011
Exif.Image.ExposureProgram                   Short       1  Manual
Exif.Image.ISOSpeedRatings                   Short       1  250
Exif.Image.0x9000                            Undefined   4  48 50 50 49
Exif.Image.DateTimeOriginal                  Ascii      20  2006:05:27 12:37:03
Exif.Image.0x9004                            Ascii      20  2006:05:27 12:37:03
Exif.Image.0x9101                            Undefined   4  0 0 0 0
Exif.Image.ShutterSpeedValue                 SRational   1  3 s
Exif.Image.ApertureValue                     Rational    1  F9
Exif.Image.ExposureBiasValue                 SRational   1  0 EV
Exif.Image.MeteringMode                      Short       1  Multi-segment
Exif.Image.Flash                             Short       1  No, compulsory
Exif.Image.FocalLength                       Rational    1  24.0 mm
Exif.Image.0x9286                            Undefined 264  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
"""]
    stderr = ["""Error: Directory Image: Next pointer is out of bounds; ignored.
"""]
    retval = [0]

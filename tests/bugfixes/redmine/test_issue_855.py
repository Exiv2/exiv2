# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class DontSegfaultWhenAccessingFocalLength(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/855"

    filename = path("$data_path/exiv2-bug855.jpg")

    commands = ["$exiv2 -pa -b $filename"]

    stdout = ["""Exif.Image.NewSubfileType                    Long        1  Thumbnail/Preview image
Exif.Image.Make                              Ascii      20  PENTAX             
Exif.Image.Model                             Ascii      20  PENTAX K-x         
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.Software                          Ascii      16  darktable 0.9.3
Exif.Image.DateTime                          Ascii      20  2012:01:22 01:54:26
Exif.Image.Artist                            Ascii      17  SEBASTIAN WAGNER
Exif.Image.Rating                            SLong       1  1
Exif.Image.RatingPercent                     SLong       1  20
Exif.Image.Copyright                         Ascii      33  CREATIVECOMMONS ATTRIBUTION CCBY
Exif.Image.ExifTag                           Long        1  666
Exif.Photo.ExposureTime                      Rational    1  1/20 s
Exif.Photo.FNumber                           Rational    1  F5.6
Exif.Photo.ExposureProgram                   Short       1  Landscape mode
Exif.Photo.ISOSpeedRatings                   Short       1  2500
Exif.Photo.DateTimeOriginal                  Ascii      20  2012:01:22 01:54:26
Exif.Photo.DateTimeDigitized                 Ascii      20  2012:01:22 01:54:26
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MeteringMode                      Short       1  Multi-segment
Exif.Photo.Flash                             Short       1  No, compulsory
Exif.Photo.FocalLength                       Rational    0  
Exif.Photo.ColorSpace                        Short       1  sRGB
Exif.Photo.SensingMethod                     Short       1  One-chip color area
Exif.Photo.CustomRendered                    Short       1  Normal process
Exif.Photo.ExposureMode                      Short       1  Auto
Exif.Photo.WhiteBalance                      Short       1  Auto
Exif.Photo.FocalLengthIn35mmFilm             Short       1  82.0 mm
Exif.Photo.SceneCaptureType                  Short       1  Landscape
Exif.Photo.Contrast                          Short       1  Hard
Exif.Photo.Saturation                        Short       1  Normal
Exif.Photo.Sharpness                         Short       1  Hard
Exif.Photo.SubjectDistanceRange              Short       1  Macro
Exif.Image.DNGVersion                        Byte        4  1 1 0 0
Exif.Image.DNGBackwardVersion                Byte        4  1 1 0 0
Exif.Image.UniqueCameraModel                 Ascii      11  PENTAX K-x
Exif.Image.ColorMatrix1                      SRational   9  71147/65536 -36788/65536 223/65536 -28867/65536 69463/65536 28977/65536 -1276/65536 2064/65536 45215/65536
Exif.Image.ColorMatrix2                      SRational   9  68429/65536 -21793/65536 -7522/65536 -36512/65536 89125/65536 14083/65536 -7905/65536 11496/65536 48817/65536
Exif.Image.AnalogBalance                     Rational    3  1/1 1/1 1/1
Exif.Image.AsShotNeutral                     Rational    3  256/352 256/264 256/762
Exif.Image.BaselineExposure                  SRational   1  -31775/65536
Exif.Image.BaselineNoise                     Rational    1  1/1
Exif.Image.BaselineSharpness                 Rational    1  1/1
Exif.Image.LinearResponseLimit               Rational    1  1/1
Exif.Image.CalibrationIlluminant1            Short       1  Standard light A
Exif.Image.CalibrationIlluminant2            Short       1  D65
"""
    ]
    stderr = [
        """Error: Upper boundary of data for directory Photo, entry 0x920a is out of bounds: Offset = 0x000003dc, size = 8, exceeds buffer size by 6 Bytes; truncating the entry
"""
    ]
    retval = [0]

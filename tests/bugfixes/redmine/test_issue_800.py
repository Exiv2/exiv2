# -*- coding: utf-8 -*-

import itertools

import system_tests


TYPES = ["8BIM", "AgHg", "DCSR", "PHUT"]
FORMATS = ["jpg", "psd"]


def make_commands(filename):
    return [
        "$exiv2 -u -v -M'set Exif.Photo.UserComment Test' " + filename,
        "$exiv2 -u -pt " + filename
    ]


def make_filename(irb_type, img_format):
    return "$data_path/exiv2-bug800-" + irb_type + "_copy." + img_format


def make_stdout(irb_type, img_format):
    first = """File 1/1: """ + make_filename(irb_type, img_format) + """
Set Exif.Photo.UserComment "Test" (Comment)
"""

    if img_format == "jpg":
        second = """Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  180
Exif.Image.YResolution                       Rational    1  180
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.Software                          Ascii      16  QuickTime 6.5.2
Exif.Image.DateTime                          Ascii      20  2005:03:27 19:46:52
Exif.Image.HostComputer                      Ascii      16  Mac OS X 10.3.8
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.ExifTag                           Long        1  190
Exif.Photo.ExposureTime                      Rational    1  1/200 s
Exif.Photo.FNumber                           Rational    1  F7.1
Exif.Photo.ExifVersion                       Undefined   4  2.20
Exif.Photo.DateTimeOriginal                  Ascii      20  2005:03:18 22:15:08
Exif.Photo.DateTimeDigitized                 Ascii      20  2005:03:18 22:15:08
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.CompressedBitsPerPixel            Rational    1  5
Exif.Photo.ShutterSpeedValue                 SRational   1  1/202 s
Exif.Photo.ApertureValue                     Rational    1  F7.1
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  F2.8
Exif.Photo.MeteringMode                      Short       1  Multi-segment
Exif.Photo.Flash                             Short       1  No, auto
Exif.Photo.FocalLength                       Rational    1  7.4 mm
Exif.Photo.MakerNote                         Undefined 590  (Binary value suppressed)
Exif.Photo.UserComment                       Undefined  12  Test
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.PixelXDimension                   Short       1  2272
Exif.Photo.PixelYDimension                   Short       1  1704
Exif.Photo.FocalPlaneXResolution             Rational    1  8114.29
Exif.Photo.FocalPlaneYResolution             Rational    1  8114.29
Exif.Photo.FocalPlaneResolutionUnit          Short       1  inch
Exif.Photo.SensingMethod                     Short       1  One-chip color area
Exif.Photo.FileSource                        Undefined   1  Digital still camera
Exif.Photo.CustomRendered                    Short       1  Normal process
Exif.Photo.ExposureMode                      Short       1  Auto
Exif.Photo.WhiteBalance                      Short       1  Auto
Exif.Photo.DigitalZoomRatio                  Rational    1  1.0
Exif.Photo.SceneCaptureType                  Short       1  Standard
"""
    elif img_format == 'psd':
        second = """Exif.Image.ImageWidth                        Short       1  150
Exif.Image.ImageLength                       Short       1  91
Exif.Image.BitsPerSample                     Short       3  8 8 8
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.SamplesPerPixel                   Short       1  3
Exif.Image.XResolution                       Rational    1  72
Exif.Image.YResolution                       Rational    1  72
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.Software                          Ascii      30  Adobe Photoshop CS5 Macintosh
Exif.Image.DateTime                          Ascii      20  2011:06:27 21:41:02
Exif.Image.ExifTag                           Long        1  218
Exif.Photo.ExifVersion                       Undefined   4  2.21
Exif.Photo.UserComment                       Undefined  12  Test
Exif.Photo.ColorSpace                        Short       1  Uncalibrated
Exif.Photo.PixelXDimension                   Long        1  150
Exif.Photo.PixelYDimension                   Long        1  91
Exif.Thumbnail.Compression                   Short       1  JPEG (old-style)
Exif.Thumbnail.XResolution                   Rational    1  72
Exif.Thumbnail.YResolution                   Rational    1  72
Exif.Thumbnail.ResolutionUnit                Short       1  inch
Exif.Thumbnail.JPEGInterchangeFormat         Long        1  390
Exif.Thumbnail.JPEGInterchangeFormatLength   Long        1  0
"""
    return [first, second]


FILES = [
    make_filename(irb_type, img_format)
    for irb_type, img_format in itertools.product(TYPES, FORMATS)
]

ORIGINAL_FILES = [
    "$data_path/exiv2-bug800-" + irb_type + "." + img_format
    for irb_type, img_format in itertools.product(TYPES, FORMATS)
]


@system_tests.CopyFiles(*ORIGINAL_FILES)
class MissingPhotoshopIrbTypes(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/800"

    types = ["8BIM", "AgHg", "DCSR", "PHUT"]
    formats = ["jpg", "psd"]

    commands = list(
        itertools.chain.from_iterable(
            make_commands(fname) for fname in FILES
        )
    )

    stdout = list(
        itertools.chain.from_iterable(
            make_stdout(irb_type, img_format)
            for irb_type, img_format in itertools.product(TYPES, FORMATS)
        )
    )
    stderr = [""] * 16
    retval = [0] * 16

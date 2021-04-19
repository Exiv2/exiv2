# -*- coding: utf-8 -*-

import itertools

import system_tests


TYPES = ["8BIM", "AgHg", "DCSR", "PHUT"]
FORMATS = ["jpg", "psd"]


def make_commands(filename):
    return [
        """$exiv2 -u -v -M"set Exif.Photo.UserComment Test" """ + filename,
        "$exiv2 -u -pt -b " + filename
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
Exif.Photo.MakerNote                         Undefined 590  14 0 1 0 3 0 46 0 0 0 92 4 0 0 2 0 3 0 4 0 0 0 184 4 0 0 3 0 3 0 4 0 0 0 192 4 0 0 4 0 3 0 34 0 0 0 200 4 0 0 0 0 3 0 6 0 0 0 12 5 0 0 0 0 3 0 4 0 0 0 24 5 0 0 18 0 3 0 28 0 0 0 32 5 0 0 19 0 3 0 4 0 0 0 88 5 0 0 6 0 2 0 32 0 0 0 96 5 0 0 7 0 2 0 24 0 0 0 128 5 0 0 8 0 4 0 1 0 0 0 104 167 20 0 9 0 2 0 32 0 0 0 152 5 0 0 16 0 4 0 1 0 0 0 0 0 39 1 13 0 3 0 34 0 0 0 184 5 0 0 0 0 0 0 92 0 2 0 0 0 5 0 1 0 0 0 0 0 4 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 15 0 3 0 1 0 1 64 0 0 255 255 255 255 199 2 237 0 32 0 98 0 180 0 0 0 0 0 0 0 0 0 0 0 0 0 255 255 0 0 224 8 224 8 0 0 1 0 0 0 0 0 255 127 0 0 0 0 0 0 2 0 237 0 30 1 215 0 0 4 0 0 0 0 0 0 68 0 0 0 128 0 50 1 181 0 245 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 121 0 0 0 180 0 247 0 0 0 0 0 0 0 250 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 9 0 9 0 224 8 168 6 224 8 212 0 153 1 38 0 102 254 0 0 154 1 102 254 0 0 154 1 102 254 0 0 154 1 215 255 215 255 215 255 0 0 0 0 0 0 41 0 41 0 41 0 16 0 4 0 0 0 0 0 0 0 0 0 73 77 71 58 80 111 119 101 114 83 104 111 116 32 83 52 48 48 32 74 80 69 71 0 0 0 0 0 0 0 0 0 70 105 114 109 119 97 114 101 32 86 101 114 115 105 111 110 32 50 46 48 48 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 68 0 9 0 67 1 57 1 62 1 65 1 69 1 68 1 67 1 66 1 70 1 64 0 0 0 0 0 83 0 0 0 0 0 10 0 0 0 10 0 3 0 120 0 123 1 26 0 249 255 249 3 10 4 0 0 0 0 0 0 0 0 153 0 0 0 0 0
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

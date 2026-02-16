# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


# Current master fails with image
class FailToReadJpgImageTest(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-bug2565.jpg")
    commands = ["$exiv2 -Pkycvt  $filename"]

    stdout = [
        """Exif.Image.ImageWidth                        Long        1  4032  4032
Exif.Image.ImageLength                       Long        1  3024  3024
Exif.Image.Make                              Ascii       8  samsung  samsung
Exif.Image.Model                             Ascii       9  SM-G930V  SM-G930V
Exif.Image.Orientation                       Short       1  6  right, top
Exif.Image.XResolution                       Rational    1  72/1  72
Exif.Image.YResolution                       Rational    1  72/1  72
Exif.Image.ResolutionUnit                    Short       1  2  inch
Exif.Image.Software                          Ascii      14  G930VVRS4BRA1  G930VVRS4BRA1
Exif.Image.DateTime                          Ascii      20  2018:04:10 18:28:03  2018:04:10 18:28:03
Exif.Image.YCbCrPositioning                  Short       1  1  Centered
Exif.Image.ExifTag                           Long        1  238  238
Exif.Photo.ExposureTime                      Rational    1  1/3224  1/3224 s
Exif.Photo.FNumber                           Rational    1  170/100  F1.7
Exif.Photo.ExposureProgram                   Short       1  2  Auto
Exif.Photo.ISOSpeedRatings                   Short       1  40  40
Exif.Photo.ExifVersion                       Undefined   4  48 50 50 48  2.20
Exif.Photo.DateTimeOriginal                  Ascii      20  2018:04:10 18:28:03  2018:04:10 18:28:03
Exif.Photo.DateTimeDigitized                 Ascii      20  2018:04:10 18:28:03  2018:04:10 18:28:03
Exif.Photo.ComponentsConfiguration           Undefined   4  1 2 3 0  YCbCr
Exif.Photo.ShutterSpeedValue                 SRational   1  11654/1000  1/3223 s
Exif.Photo.ApertureValue                     Rational    1  153/100  F1.7
Exif.Photo.BrightnessValue                   SRational   1  946/100  9.46
Exif.Photo.ExposureBiasValue                 SRational   1  0/10  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  153/100  F1.7
Exif.Photo.MeteringMode                      Short       1  2  Center weighted average
Exif.Photo.LightSource                       Short       1  0  Unknown
Exif.Photo.Flash                             Short       1  0  No flash
Exif.Photo.FocalLength                       Rational    1  420/100  4.2 mm
Exif.Photo.MakerNote                         Undefined  98  7 0 1 0 7 0 4 0 0 0 48 49 48 48 2 0 4 0 1 0 0 0 0 32 1 0 12 0 4 0 1 0 0 0 0 0 0 0 16 0 5 0 1 0 0 0 90 0 0 0 64 0 4 0 1 0 0 0 0 0 0 0 80 0 4 0 1 0 0 0 1 0 0 0 0 1 3 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  7 0 1 0 7 0 4 0 0 0 48 49 48 48 2 0 4 0 1 0 0 0 0 32 1 0 12 0 4 0 1 0 0 0 0 0 0 0 16 0 5 0 1 0 0 0 90 0 0 0 64 0 4 0 1 0 0 0 0 0 0 0 80 0 4 0 1 0 0 0 1 0 0 0 0 1 3 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
Exif.Photo.UserComment                       Undefined 4980  charset=Ascii 
  charset=Ascii 

Exif.Photo.FlashpixVersion                   Undefined   4  48 49 48 48  1.00
Exif.Photo.ColorSpace                        Short       1  1  sRGB
Exif.Photo.PixelXDimension                   Long        1  4032  4032
Exif.Photo.PixelYDimension                   Long        1  3024  3024
Exif.Photo.InteroperabilityTag               Long        1  5810  5810
Exif.Iop.InteroperabilityIndex               Ascii       4  R98  R98
Exif.Iop.InteroperabilityVersion             Undefined   4  48 49 48 48  1.00
Exif.Photo.SensingMethod                     Short       1  2  One-chip color area
Exif.Photo.SceneType                         Undefined   1  1  Directly photographed
Exif.Photo.ExposureMode                      Short       1  0  Auto
Exif.Photo.WhiteBalance                      Short       1  0  Auto
Exif.Photo.FocalLengthIn35mmFilm             Short       1  26  26.0 mm
Exif.Photo.SceneCaptureType                  Short       1  0  Standard
Exif.Photo.ImageUniqueID                     Ascii      11  C12QLJK01SM  C12QLJK01SM
Exif.Image.GPSTag                            Long        1  5840  5840
Exif.GPSInfo.GPSVersionID                    Byte        4  2 2 0 0  2.2.0.0
Exif.GPSInfo.GPSLatitudeRef                  Ascii       2  N  North
Exif.GPSInfo.GPSLatitude                     Rational    3  37/1 47/1 97961/10000  37 deg 47' 9.80"
Exif.GPSInfo.GPSLongitudeRef                 Ascii       2  W  West
Exif.GPSInfo.GPSLongitude                    Rational    3  122/1 23/1 275107/10000  122 deg 23' 27.51"
Exif.GPSInfo.GPSAltitudeRef                  Byte        1  0  Above sea level
Exif.GPSInfo.GPSAltitude                     Rational    1  0/1000  0.0 m
Exif.GPSInfo.GPSTimeStamp                    Rational    3  1/1 27/1 52/1  01:27:52
Exif.GPSInfo.GPSDateStamp                    Ascii      11  2018:04:11  2018:04:11
Exif.Thumbnail.ImageWidth                    Long        1  504  504
Exif.Thumbnail.ImageLength                   Long        1  376  376
Exif.Thumbnail.Compression                   Short       1  6  JPEG (old-style)
Exif.Thumbnail.Orientation                   Short       1  6  right, top
Exif.Thumbnail.XResolution                   Rational    1  72/1  72
Exif.Thumbnail.YResolution                   Rational    1  72/1  72
Exif.Thumbnail.ResolutionUnit                Short       1  2  inch
Exif.Thumbnail.JPEGInterchangeFormat         Long        1  6176  6176
Exif.Thumbnail.JPEGInterchangeFormatLength   Long        1  13544  13544
"""
    ]
    stderr = [""]
    retval = [0]

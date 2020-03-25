# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-bug528.jpg")
class TypeSizeForExifOnly(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/528"

    filename = "$data_path/exiv2-bug528_copy.jpg"

    commands = [
        "$exiv2 -u -pt $filename",
        """$exiv2 -u -v -M"set Exif.Image.Software GI" $filename"""
    ]

    stdout = ["""Exif.Image.ProcessingSoftware                Ascii      20  digiKam-0.9.4-beta4
Exif.Image.ImageWidth                        Long        1  2888
Exif.Image.ImageLength                       Long        1  1700
Exif.Image.DocumentName                      Ascii      13  Chambord.jpg
Exif.Image.Make                              Ascii       6  Canon
Exif.Image.Model                             Ascii      14  Canon EOS 40D
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  72
Exif.Image.YResolution                       Rational    1  72
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.Software                          Ascii      11  GIMP 2.4.5
Exif.Image.DateTime                          Ascii      20  2008:04:14 08:24:04
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.Rating                            SLong       1  3
Exif.Image.RatingPercent                     SLong       1  50
Exif.Image.ExifTag                           Long        1  318
Exif.Photo.ExposureTime                      Rational    1  1/250 s
Exif.Photo.FNumber                           Rational    1  F11
Exif.Photo.ExposureProgram                   Short       1  Auto
Exif.Photo.ISOSpeedRatings                   Short       1  400
Exif.Photo.ExifVersion                       Undefined   4  2.21
Exif.Photo.DateTimeOriginal                  Ascii      20  2008:04:01 13:40:03
Exif.Photo.DateTimeDigitized                 Ascii      20  2008:04:01 13:40:03
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.ShutterSpeedValue                 SRational   1  1/256 s
Exif.Photo.ApertureValue                     Rational    1  F11
Exif.Photo.ExposureBiasValue                 SRational   1  +1/3 EV
Exif.Photo.MeteringMode                      Short       1  Multi-segment
Exif.Photo.Flash                             Short       1  No, compulsory
Exif.Photo.FocalLength                       Rational    1  10.0 mm
Exif.Photo.MakerNote                         Undefined  18  0 1 2 0 3 0 0 0 0 0 0 0 0 0 0 0 0 0
Exif.MakerNote.Offset                        Long        1  796
Exif.MakerNote.ByteOrder                     Ascii       3  MM
Exif.Canon.0x0200                            0x0300      0  
Exif.Photo.UserComment                       Undefined  37  Chateaux de la Loire
Chambord
Exif.Photo.SubSecTime                        Ascii       3  81
Exif.Photo.SubSecTimeOriginal                Ascii       3  81
Exif.Photo.SubSecTimeDigitized               Ascii       3  81
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ColorSpace                        Short       1  sRGB
Exif.Photo.PixelXDimension                   Long        1  2888
Exif.Photo.PixelYDimension                   Long        1  1700
Exif.Photo.FocalPlaneXResolution             Rational    1  4438.36
Exif.Photo.FocalPlaneYResolution             Rational    1  4445.97
Exif.Photo.FocalPlaneResolutionUnit          Short       1  inch
Exif.Photo.CustomRendered                    Short       1  Normal process
Exif.Photo.ExposureMode                      Short       1  Auto
Exif.Photo.WhiteBalance                      Short       1  Auto
Exif.Photo.SceneCaptureType                  Short       1  Standard
Exif.Photo.Contrast                          Short       1  Normal
Exif.Photo.Saturation                        Short       1  Normal
Exif.Image.GPSTag                            Long        1  867
Exif.GPSInfo.GPSVersionID                    Byte        4  2.0.0.0
Exif.GPSInfo.GPSLatitudeRef                  Ascii       2  North
Exif.GPSInfo.GPSLatitude                     Rational    3  47deg 36' 58"
Exif.GPSInfo.GPSLongitudeRef                 Ascii       2  East
Exif.GPSInfo.GPSLongitude                    Rational    3  1deg 31' 1"
Exif.GPSInfo.GPSAltitudeRef                  Byte        1  Above sea level
Exif.GPSInfo.GPSAltitude                     Rational    1  86 m
Exif.Thumbnail.Compression                   Short       1  JPEG (old-style)
Exif.Thumbnail.JPEGInterchangeFormat         Long        1  1055
Exif.Thumbnail.JPEGInterchangeFormatLength   Long        1  4492
""",
        """File 1/1: $filename
Set Exif.Image.Software "GI" (Ascii)
"""
    ]
    stderr = [
        """Warning: Directory Canon, entry 0x0200 has unknown Exif (TIFF) type 768; setting type size 1.
""",
        """Warning: Directory Canon, entry 0x0200 has unknown Exif (TIFF) type 768; setting type size 1.
Warning: Directory Canon, entry 0x0200 has unknown Exif (TIFF) type 768; setting type size 1.
"""
    ]
    retval = [0] * 2

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors

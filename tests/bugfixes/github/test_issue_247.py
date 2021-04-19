# -*- coding: utf-8 -*-

import system_tests

class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the first bug described in:
    https://github.com/Exiv2/exiv2/issues/247
    """
    url = "https://github.com/Exiv2/exiv2/issues/247"

    filename = "$data_path/2-invalid-memory-access"
    commands = ["$exiv2 -pt -b " + filename]
    stdout = [
        """Exif.Image.Make                              Ascii       6  Canon
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  180
Exif.Image.YResolution                       Rational    1  180
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.DateTime                          Ascii      20  2003:12:14 12:01:44
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.ExifTag                           Long        1  196
Exif.Photo.ExposureTime                      Rational    1  1/500 s
Exif.Photo.FNumber                           Rational    1  F4.9
Exif.Photo.ExifVersion                       Undefined   4  2.20
Exif.Photo.DateTimeOriginal                  Ascii      20  2003:12:14 12:01:44
Exif.Photo.DateTimeDigitized                 Ascii      20  2003:12:14 12:01:44
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.CompressedBitsPerPixel            Rational    1  5
Exif.Photo.ShutterSpeedValue                 SRational   1  1/501 s
Exif.Photo.ApertureValue                     Rational    1  F5
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  F2.8
Exif.Photo.MeteringMode                      Short       1  Center weighted average
Exif.Photo.Flash                             Short       1  No, auto
Exif.Photo.FocalLength                       Rational    1  21.3 mm
Exif.Photo.MakerNote                         Undefined 450  12 0 1 0 3 0 40 0 0 0 68 4 0 0 2 0 3 0 4 0 0 0 148 4 0 0 3 0 3 0 4 0 0 0 156 4 0 0 4 0 3 0 27 0 0 0 164 4 0 0 0 0 3 0 6 0 0 0 218 4 0 0 0 0 3 0 4 0 0 0 230 4 0 0 6 0 2 0 32 0 0 0 238 4 0 0 7 0 2 0 24 0 0 0 14 5 0 0 8 0 4 0 1 0 0 0 59 225 17 0 9 0 2 0 32 0 0 0 38 5 0 0 16 0 4 0 1 0 0 0 0 0 17 1 13 0 3 0 21 0 0 0 70 5 0 0 0 0 0 0 80 0 2 0 0 0 5 0 1 0 0 0 0 0 4 0 0 0 1 0 0 0 1 0 0 0 0 0 0 0 0 0 17 0 5 0 1 0 3 48 1 0 255 255 255 255 170 2 227 0 32 0 149 0 192 0 0 0 0 0 0 0 0 0 0 0 0 0 255 255 49 0 224 8 224 8 0 0 1 0 2 0 170 2 30 1 215 0 0 0 0 0 0 0 0 0 54 0 0 0 160 0 20 1 149 0 31 1 0 0 0 0 0 0 0 0 6 0 0 0 0 0 0 0 2 48 0 0 0 0 0 0 1 0 14 3 0 0 149 0 33 1 0 0 0 0 0 0 250 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 73 77 71 58 80 111 119 101 114 83 104 111 116 32 83 52 48 32 74 80 69 71 0 0 0 0 0 0 0 0 0 0 70 105 114 109 119 97 114 101 32 86 101 114 115 105 111 110 32 49 46 49 48 0 0 0 65 110 100 114 101 97 115 32 72 117 103 103 101 108 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 42 0 3 0 1 128 122 1 1 128 0 0 0 0 0 0 3 1 2 0 0 0 10 0 0 0 0 0 0 0 57 0 198 0 5 0 0 0 0 0 0 0
Exif.MakerNote.Offset                        Long        1  942
Exif.MakerNote.ByteOrder                     Ascii       3  II
Exif.CanonCs.Macro                           Short       1  Off
Exif.CanonCs.Selftimer                       Short       1  Off
Exif.CanonCs.Quality                         Short       1  Superfine
Exif.CanonCs.FlashMode                       Short       1  Auto
Exif.CanonCs.DriveMode                       Short       1  Single / timer
Exif.CanonCs.FocusMode                       Short       1  Single
Exif.CanonCs.RecordMode                      Short       1  JPEG
Exif.CanonCs.ImageSize                       Short       1  Large
Exif.CanonCs.EasyMode                        Short       1  Manual
Exif.CanonCs.DigitalZoom                     Short       1  None
Exif.CanonCs.Contrast                        Short       1  Normal
Exif.CanonCs.Saturation                      Short       1  Normal
Exif.CanonCs.Sharpness                       Short       1  Normal
Exif.CanonCs.ISOSpeed                        Short       1  100
Exif.CanonCs.MeteringMode                    Short       1  Center-weighted average
Exif.CanonCs.FocusType                       Short       1  Auto
Exif.CanonCs.AFPoint                         Short       1  Center
Exif.CanonCs.ExposureProgram                 Short       1  Program (P)
Exif.CanonCs.LensType                        Short       1  n/a
Exif.CanonCs.Lens                            Short       3  7.1 - 21.3 mm
Exif.CanonCs.MaxAperture                     Short       1  F5
Exif.CanonCs.MinAperture                     Short       1  F8
Exif.CanonCs.FlashActivity                   Short       1  Did not fire
Exif.CanonCs.FlashDetails                    Short       1  
Exif.CanonCs.FocusContinuous                 Short       1  Single
Exif.CanonCs.AESetting                       Short       1  Normal AE
Exif.CanonCs.ImageStabilization              Short       1  (65535)
Exif.CanonCs.DisplayAperture                 Short       1  49
Exif.CanonCs.ZoomSourceWidth                 Short       1  2272
Exif.CanonCs.ZoomTargetWidth                 Short       1  2272
Exif.CanonCs.SpotMeteringMode                Short       1  AF Point
Exif.Canon.FocalLength                       Short       4  21.3 mm
Exif.CanonSi.AutoISO                         Short       1  100
Exif.CanonSi.ISOSpeed                        Short       1  100
Exif.CanonSi.MeasuredEV                      Short       1  13.63
Exif.CanonSi.TargetAperture                  Short       1  F5
Exif.CanonSi.TargetShutterSpeed              Short       1  1/501 s
Exif.CanonSi.WhiteBalance                    Short       1  Auto
Exif.CanonSi.SlowShutter                     Short       1  Off
Exif.CanonSi.Sequence                        Short       1  0
Exif.CanonSi.CameraTemperature               Short       1  --
Exif.CanonSi.FlashGuideNumber                Short       1  0
Exif.CanonSi.AFPointUsed                     Short       1  3 focus points; center used
Exif.CanonSi.FlashBias                       Short       1  0 EV
Exif.CanonSi.AutoExposureBracketing          Short       1  Off
Exif.CanonSi.SubjectDistance                 Short       1  7.82 m
Exif.CanonSi.ApertureValue                   Short       1  F5
Exif.CanonSi.ShutterSpeedValue               Short       1  1/523 s
Exif.CanonSi.MeasuredEV2                     Short       1  -6.00
Exif.CanonSi.BulbDuration                    Short       1  0
Exif.CanonSi.CameraType                      Short       1  Compact
Exif.Canon.ImageType                         Ascii      32  IMG:PowerShot S40 JPEG
Exif.Canon.FirmwareVersion                   Ascii      24  Firmware Version 1.10
Exif.Canon.FileNumber                        Long        1  117-1771
Exif.Canon.OwnerName                         Ascii      32  Andreas Huggel
Exif.Canon.ModelID                           Long        1  PowerShot S40
Exif.Canon.CameraInfo                        Short      21  42 3 32769 378 32769 0 0 0 259 2 0 10 0 0 0 57 198 5 0 0 0
Exif.Photo.UserComment                       Undefined 264  
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ColorSpace                        Short       1  sRGB
Exif.Photo.PixelXDimension                   Short       1  2272
Exif.Photo.PixelYDimension                   Short       1  1704
Exif.Photo.InteroperabilityTag               Long        1  1416
Exif.Iop.InteroperabilityIndex               Ascii       4  R98
Exif.Iop.InteroperabilityVersion             Undefined   4  1.00
Exif.Iop.RelatedImageWidth                   Short       1  2272
Exif.Iop.RelatedImageLength                  Short       1  1704
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
Exif.Thumbnail.Compression                   Short       1  JPEG (old-style)
Exif.Thumbnail.XResolution                   Rational    1  180
Exif.Thumbnail.YResolution                   Rational    1  180
Exif.Thumbnail.ResolutionUnit                Short       1  inch
Exif.Thumbnail.JPEGInterchangeFormat         Long        1  2036
Exif.Thumbnail.JPEGInterchangeFormatLength   Long        1  5448
"""]
    stderr = [""]
    retval = [0]

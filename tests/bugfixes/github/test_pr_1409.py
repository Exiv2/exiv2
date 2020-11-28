# -*- coding: utf-8 -*-

import system_tests

class FujiFilm_IFD_Tags_pr1409(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1409"
    
    filename = "$data_path/exiv2-pr1409.exv"
    commands  = ["$exiv2 -g Fujifilm $filename"]
    stderr = [""]
    stdout = ["""Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FF02B4550110     593534373734180817925330110168
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  Normal
Exif.Fujifilm.FlashMode                      Short       1  (38976)
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.FocusMode                      Short       1  Auto
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Aperture-priority AE
Exif.Fujifilm.ShadowTone                     SLong       1  0
Exif.Fujifilm.HighlightTone                  SLong       1  0
Exif.Fujifilm.CropMode                       Short       1  None
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  Off
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.FilmMode                       Short       1  F0/Standard (Provia)
Exif.Fujifilm.DynamicRangeSetting            Short       1  Raw
Exif.Fujifilm.DevelopmentDynamicRange        Short       1  100
Exif.Fujifilm.MinFocalLength                 Rational    1  1800/100
Exif.Fujifilm.MaxFocalLength                 Rational    1  5500/100
Exif.Fujifilm.MaxApertureAtMinFocal          Rational    1  280/100
Exif.Fujifilm.MaxApertureAtMaxFocal          Rational    1  400/100
Exif.Fujifilm.Rating                         Long        1  0
Exif.Fujifilm.RawImageFullWidth              Long        1  6384
Exif.Fujifilm.RawImageFullHeight             Long        1  4182
Exif.Fujifilm.BitsPerSample                  Long        1  14
Exif.Fujifilm.StripOffsets                   Long        1  2048
Exif.Fujifilm.StripByteCounts                Long        1  53395776
Exif.Fujifilm.BlackLevel                     Long       36  1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022 1022
Exif.Fujifilm.GeometricDistortionParams      SRational  19  3749/9 502/1420 710/1420 870/1420 1004/1420 1123/1420 1230/1420 1328/1420 1420/1420 1506/1420 35789/65536 64000/65536 92105/65536 121578/65536 152315/65536 183578/65536 216947/65536 251473/65536 286000/65536
Exif.Fujifilm.WB_GRBLevelsStandard           Long        8  302 371 826 17 302 626 485 21
Exif.Fujifilm.WB_GRBLevelsAuto               Long        3  302 561 552
Exif.Fujifilm.WB_GRBLevels                   Long        3  302 561 552
Exif.Fujifilm.ChromaticAberrationParams      SRational  29  3749/9 502/1420 710/1420 870/1420 1004/1420 1123/1420 1230/1420 1328/1420 1420/1420 1506/1420 4/65536 2/65536 -2/65536 -6/65536 -8/65536 -12/65536 -14/65536 -18/65536 -18/65536 -58/65536 -54/65536 -50/65536 -46/65536 -40/65536 -32/65536 -24/65536 -16/65536 -16/65536 3749/9
Exif.Fujifilm.VignettingParams               SRational  19  3749/9 502/1420 710/1420 870/1420 1004/1420 1123/1420 1230/1420 1328/1420 1420/1420 1506/1420 192752/2048 183960/2048 176751/2048 171126/2048 158275/2048 138976/2048 119700/2048 101213/2048 87673/2048
"""]
    retval = [0]

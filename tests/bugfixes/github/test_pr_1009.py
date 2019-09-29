# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Sony2010eTest(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr1009.exv")
    commands = ["$exiv2 -pa --grep Sony $filename"]

    stdout = ["""Exif.Sony1.Panorama                          Long       16  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
Exif.Sony1.Contrast                          SLong       1  0
Exif.Sony1.Saturation                        SLong       1  0
Exif.Sony1.AutoHDR                           Long        1  Off
Exif.Sony1.ShotInfo                          Undefined 390  (Binary value suppressed)
Exif.Sony1.ColorReproduction                 Ascii      16  Standard
Exif.Sony1.ColorTemperature                  Long        1  0
Exif.Sony1.ColorCompensationFilter           Long        1  0
Exif.Sony1.SceneMode                         Long        1  (22)
Exif.Sony1.ZoneMatching                      Long        1  ISO Setting Used
Exif.Sony1.DynamicRangeOptimizer             Long        1  Auto
Exif.Sony1.ImageStabilization                Long        1  On
Exif.Sony1.ColorMode                         Long        1  Standard
Exif.Sony1.FullImageSize                     Long        2  3264 x 4912
Exif.Sony1.PreviewImageSize                  Long        2  1080 x 1616
Exif.Sony1.FileFormat                        Byte        4  ARW 2.3.1
Exif.Sony1.Quality                           Long        1  Fine
Exif.Sony1.FlashExposureComp                 SRational   1  0 EV
Exif.Sony1.WhiteBalanceFineTune              Long        1  0
Exif.Sony1.WhiteBalance                      Long        1  Auto
Exif.Sony1.SonyModelID                       Short       1  NEX-6
Exif.Sony1.Teleconverter                     Long        1  None
Exif.Sony1.LensID                            Long        1  Manual lens
Exif.Sony1.ExposureMode                      Short       1  Program
Exif.Sony1.JPEGQuality                       Short       1  n/a
Exif.Sony1.FlashLevel                        SShort      1  Normal
Exif.Sony1.ReleaseMode                       Short       1  Normal
Exif.Sony1.SequenceNumber                    Short       1  Single
Exif.Sony1.AntiBlur                          Short       1  On (Shooting)
Exif.Sony1.DynamicRangeOptimizer             Short       1  Standard
Exif.Sony1.IntelligentAuto                   Short       1  On
Exif.Sony1.WhiteBalance2                     Short       1  Auto
Exif.Sony2Fp.AmbientTemperature              SByte       1  244
Exif.Sony2Fp.FocusMode                       Byte        1  26
Exif.Sony2Fp.AFAreaMode                      Byte        1  222
Exif.Sony2Fp.FocusPosition2                  Byte        1  168
Exif.Sony2010e.SequenceImageNumber           Long        1  0
Exif.Sony2010e.SequenceFileNumber            Long        1  0
Exif.Sony2010e.ReleaseMode2                  Long        1  0
Exif.Sony2010e.DigitalZoomRatio              Byte        1  217
Exif.Sony2010e.SonyDateTime                  Undefined   1  231
Exif.Sony2010e.DynamicRangeOptimizer         Byte        1  1
Exif.Sony2010e.MeterInfo                     Undefined   1  3
Exif.Sony2010e.ReleaseMode3                  Byte        1  0
Exif.Sony2010e.ReleaseMode2                  Byte        1  0
Exif.Sony2010e.SelfTimer                     Byte        1  1
Exif.Sony2010e.FlashMode                     Byte        1  1
Exif.Sony2010e.StopsAboveBaseISO             Short       1  206
Exif.Sony2010e.BrightnessValue               Short       1  39323
Exif.Sony2010e.DynamicRangeOptimizer         Byte        1  1
Exif.Sony2010e.HDRSetting                    Byte        1  0
Exif.Sony2010e.ExposureCompensation          SShort      1  0
Exif.Sony2010e.PictureProfile                Byte        1  0
Exif.Sony2010e.PictureProfile2               Byte        1  0
Exif.Sony2010e.PictureEffect2                Byte        1  0
Exif.Sony2010e.Quality2                      Byte        1  0
Exif.Sony2010e.MeteringMode                  Byte        1  0
Exif.Sony2010e.ExposureProgram               Byte        1  28
Exif.Sony2010e.WB_RGBLevels                  Short       3  49497 1 49921
Exif.Sony2010e.SonyISO                       Short       1  206
Exif.Sony2010e.SonyISO2                      Short       1  671
Exif.Sony2010e.FocalLength                   Short       1  236
Exif.Sony2010e.MinFocalLength                Short       1  226
Exif.Sony2010e.MaxFocalLength                Short       1  48896
Exif.Sony2010e.SonyISO3                      Short       1  41420
Exif.Sony2010e.DistortionCorrParams          SShort     16  -3584 -1793 25855 -30722 -9730 -25603 27900 -26629 250 -17928 -11360 -11360 -11360 -11360 -11360 -11360
Exif.Sony2010e.LensFormat                    Byte        1  1
Exif.Sony2010e.LensMount                     Byte        1  89
Exif.Sony2010e.LensType2                     Short       1  16527
Exif.Sony2010e.LensType                      Short       1  0
Exif.Sony2010e.DistortionCorrParamsPresent   Byte        1  1
Exif.Sony2010e.DistortionCorrParamsNumber    Byte        1  206
"""
    ]
    stderr = [""]
    retval = [0]

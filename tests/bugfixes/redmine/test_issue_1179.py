# -*- coding: utf-8 -*-

import itertools
import system_tests

def char_range(c1, c2):
    """Generates the characters from `c1` to `c2`, inclusive."""
    for c in range(ord(c1), ord(c2)+1):
        yield chr(c)

ORIGINAL_FILES = ["$data_path/exiv2-bug1179" + case + ".exv"
    for case in char_range('a', 'j')]

def make_command(filename):
    return ["$exiv2 -pa --grep fuji/i " + filename]


class CheckFilmMode(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1179"

    commands = list(itertools.chain.from_iterable(
            make_command(fname) for fname in ORIGINAL_FILES))

    stdout = [
        """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  Normal
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Auto
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  Off
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.FilmMode                       Short       1  F0/Standard (Provia)
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
        """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  Normal
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Auto
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  Off
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.FilmMode                       Short       1  F2/Fujichrome (Velvia)
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""", #b
        """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  Normal
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Auto
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  Off
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.FilmMode                       Short       1  F1b/Studio Portrait Smooth Skin Tone (Astia)
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  Normal
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Auto
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  Off
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.FilmMode                       Short       1  Classic Chrome
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  Normal
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Auto
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  Off
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.FilmMode                       Short       1  Pro Neg. Hi
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  None (black & white)
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Manual
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  On
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  On
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  (770)
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Manual
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Program AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  On
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  On
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  (769)
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Manual
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Shutter speed priority AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  On
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  (771)
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Manual
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Shutter speed priority AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  On
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
""",
    """Exif.Fujifilm.Version                        Undefined   4  48 49 51 48
Exif.Fujifilm.SerialNumber                   Ascii      48  FFDT22794526     593332303134151113535030217060
Exif.Fujifilm.Quality                        Ascii       8  NORMAL 
Exif.Fujifilm.Sharpness                      Short       1  Normal
Exif.Fujifilm.WhiteBalance                   Short       1  Auto
Exif.Fujifilm.Color                          Short       1  (784)
Exif.Fujifilm.FlashMode                      Short       1  Off
Exif.Fujifilm.FlashStrength                  SRational   1  0/100
Exif.Fujifilm.Macro                          Short       1  Off
Exif.Fujifilm.FocusMode                      Short       1  Manual
Exif.Fujifilm.SlowSync                       Short       1  Off
Exif.Fujifilm.PictureMode                    Short       1  Shutter speed priority AE
Exif.Fujifilm.Continuous                     Short       1  Off
Exif.Fujifilm.SequenceNumber                 Short       1  0
Exif.Fujifilm.BlurWarning                    Short       1  Off
Exif.Fujifilm.FocusWarning                   Short       1  Off
Exif.Fujifilm.ExposureWarning                Short       1  On
Exif.Fujifilm.DynamicRange                   Short       1  Standard
Exif.Fujifilm.DynamicRangeSetting            Short       1  Auto (100-400%)
"""
    ]
    stderr = [""] * 10
    retval = [0] * 10

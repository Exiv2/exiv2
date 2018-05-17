# -*- coding: utf-8 -*-

import system_tests


URL = "http://dev.exiv2.org/issues/711"

COMMANDS = [
    """$exiv2 -u -v -M"set Exif.Image.ProcessingSoftware Initial values, read from the command line" -M"set Exif.Image.DocumentName Float 0.12345" -M"set Exif.Image.ImageDescription Double 0.987654321" $filename""",

    "$exiv2 -u -v -PEkyct $filename",
    """$exiv2 -u -v -M"set Exif.Image.ProcessingSoftware Non-intrusive update" $filename""",
    "$exiv2 -u -v -PEkyct $filename",
    """$exiv2 -u -v -M"set Exif.Image.ProcessingSoftware Intrusive update, writing the structure from scratch" $filename""",
    "$exiv2 -u -v -PEkyct $filename",
]



@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class LittleEndianORFFilesFromE_PL1Corrupted(metaclass=system_tests.CaseMeta):

    filename = "$data_path/exiv2-empty_copy.jpg"

    commands = COMMANDS

    stdout = [
        """File 1/1: $filename
Set Exif.Image.ProcessingSoftware "Initial values, read from the command line" (Ascii)
Set Exif.Image.DocumentName "0.12345" (Float)
Set Exif.Image.ImageDescription "0.987654321" (Double)
""",
        """File 1/1: $filename
Exif.Image.ProcessingSoftware                Ascii      43  Initial values, read from the command line
Exif.Image.DocumentName                      Float       1  0.123450003564358
Exif.Image.ImageDescription                  Double      1  0.987654321
""",
        """File 1/1: $filename
Set Exif.Image.ProcessingSoftware "Non-intrusive update" (Ascii)
""",
        """File 1/1: $filename
Exif.Image.ProcessingSoftware                Ascii      21  Non-intrusive update
Exif.Image.DocumentName                      Float       1  0.123450003564358
Exif.Image.ImageDescription                  Double      1  0.987654321
""",
        """File 1/1: $filename
Set Exif.Image.ProcessingSoftware "Intrusive update, writing the structure from scratch" (Ascii)
""",
        """File 1/1: $filename
Exif.Image.ProcessingSoftware                Ascii      53  Intrusive update, writing the structure from scratch
Exif.Image.DocumentName                      Float       1  0.123450003564358
Exif.Image.ImageDescription                  Double      1  0.987654321
"""
    ]
    stderr = [""] * 6
    retval = [0] * 6


@system_tests.CopyFiles("$data_path/exiv2-kodak-dc210.jpg")
class BigEndianORFFilesFromE_PL1Corrupted(metaclass=system_tests.CaseMeta):

    filename = "$data_path/exiv2-kodak-dc210_copy.jpg"

    commands = COMMANDS

    stdout = [
        """File 1/1: $filename
Set Exif.Image.ProcessingSoftware "Initial values, read from the command line" (Ascii)
Set Exif.Image.DocumentName "0.12345" (Float)
Set Exif.Image.ImageDescription "0.987654321" (Double)
""",
        """File 1/1: $filename
Exif.Image.ProcessingSoftware                Ascii      43  Initial values, read from the command line
Exif.Image.DocumentName                      Float       1  0.123450003564358
Exif.Image.ImageDescription                  Double      1  0.987654321
Exif.Image.Make                              Ascii      22  Eastman Kodak Company
Exif.Image.Model                             Ascii      20  DC210 Zoom (V05.00)
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  216
Exif.Image.YResolution                       Rational    1  216
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.Copyright                         Ascii       0  
Exif.Image.ExifTag                           Long        1  268
Exif.Photo.ExposureTime                      Rational    1  1/30 s
Exif.Photo.FNumber                           Rational    1  F4
Exif.Photo.ExifVersion                       Undefined   4  1.10
Exif.Photo.DateTimeOriginal                  Ascii      20  2000:10:26 16:46:51
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.CompressedBitsPerPixel            Rational    1  (0/0)
Exif.Photo.ShutterSpeedValue                 SRational   1  1/32 s
Exif.Photo.ApertureValue                     Rational    1  F4
Exif.Photo.BrightnessValue                   SRational   1  1.5
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  F4
Exif.Photo.SubjectDistance                   Rational    1  Unknown
Exif.Photo.MeteringMode                      Short       1  Center weighted average
Exif.Photo.LightSource                       Short       1  Unknown
Exif.Photo.Flash                             Short       1  Fired
Exif.Photo.FocalLength                       Rational    1  4.4 mm
Exif.Photo.MakerNote                         Undefined  72  1 4 3 0 2 1 255 255 0 1 55 142 14 93 109 246 1 0 0 251 0 4 255 32 0 0 40 0 0 0 12 53 68 67 80 48 49 48 49 49 46 74 80 71 0 4 0 0 0 1 228 224 0 1 0 0 0 1 145 128 0 1 194 64 0 1 0 0 0 1 222 16
Exif.Thumbnail.ImageWidth                    Short       1  96
Exif.Thumbnail.ImageLength                   Short       1  72
Exif.Thumbnail.BitsPerSample                 Short       3  8 8 8
Exif.Thumbnail.Compression                   Short       1  Uncompressed
Exif.Thumbnail.PhotometricInterpretation     Short       1  RGB
Exif.Thumbnail.StripOffsets                  Short       1  822
Exif.Thumbnail.SamplesPerPixel               Short       1  3
Exif.Thumbnail.RowsPerStrip                  Short       1  72
Exif.Thumbnail.StripByteCounts               Short       1  20736
Exif.Thumbnail.XResolution                   Rational    1  72
Exif.Thumbnail.YResolution                   Rational    1  72
Exif.Thumbnail.ResolutionUnit                Short       1  inch
""",
        """File 1/1: $filename
Set Exif.Image.ProcessingSoftware "Non-intrusive update" (Ascii)
""",
        """File 1/1: $filename
Exif.Image.ProcessingSoftware                Ascii      21  Non-intrusive update
Exif.Image.DocumentName                      Float       1  0.123450003564358
Exif.Image.ImageDescription                  Double      1  0.987654321
Exif.Image.Make                              Ascii      22  Eastman Kodak Company
Exif.Image.Model                             Ascii      20  DC210 Zoom (V05.00)
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  216
Exif.Image.YResolution                       Rational    1  216
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.Copyright                         Ascii       0  
Exif.Image.ExifTag                           Long        1  268
Exif.Photo.ExposureTime                      Rational    1  1/30 s
Exif.Photo.FNumber                           Rational    1  F4
Exif.Photo.ExifVersion                       Undefined   4  1.10
Exif.Photo.DateTimeOriginal                  Ascii      20  2000:10:26 16:46:51
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.CompressedBitsPerPixel            Rational    1  (0/0)
Exif.Photo.ShutterSpeedValue                 SRational   1  1/32 s
Exif.Photo.ApertureValue                     Rational    1  F4
Exif.Photo.BrightnessValue                   SRational   1  1.5
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  F4
Exif.Photo.SubjectDistance                   Rational    1  Unknown
Exif.Photo.MeteringMode                      Short       1  Center weighted average
Exif.Photo.LightSource                       Short       1  Unknown
Exif.Photo.Flash                             Short       1  Fired
Exif.Photo.FocalLength                       Rational    1  4.4 mm
Exif.Photo.MakerNote                         Undefined  72  1 4 3 0 2 1 255 255 0 1 55 142 14 93 109 246 1 0 0 251 0 4 255 32 0 0 40 0 0 0 12 53 68 67 80 48 49 48 49 49 46 74 80 71 0 4 0 0 0 1 228 224 0 1 0 0 0 1 145 128 0 1 194 64 0 1 0 0 0 1 222 16
Exif.Thumbnail.ImageWidth                    Short       1  96
Exif.Thumbnail.ImageLength                   Short       1  72
Exif.Thumbnail.BitsPerSample                 Short       3  8 8 8
Exif.Thumbnail.Compression                   Short       1  Uncompressed
Exif.Thumbnail.PhotometricInterpretation     Short       1  RGB
Exif.Thumbnail.StripOffsets                  Short       1  822
Exif.Thumbnail.SamplesPerPixel               Short       1  3
Exif.Thumbnail.RowsPerStrip                  Short       1  72
Exif.Thumbnail.StripByteCounts               Short       1  20736
Exif.Thumbnail.XResolution                   Rational    1  72
Exif.Thumbnail.YResolution                   Rational    1  72
Exif.Thumbnail.ResolutionUnit                Short       1  inch
""",
        """File 1/1: $filename
Set Exif.Image.ProcessingSoftware "Intrusive update, writing the structure from scratch" (Ascii)
""",
        """File 1/1: $filename
Exif.Image.ProcessingSoftware                Ascii      53  Intrusive update, writing the structure from scratch
Exif.Image.DocumentName                      Float       1  0.123450003564358
Exif.Image.ImageDescription                  Double      1  0.987654321
Exif.Image.Make                              Ascii      22  Eastman Kodak Company
Exif.Image.Model                             Ascii      20  DC210 Zoom (V05.00)
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  216
Exif.Image.YResolution                       Rational    1  216
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.Copyright                         Ascii       0  
Exif.Image.ExifTag                           Long        1  278
Exif.Photo.ExposureTime                      Rational    1  1/30 s
Exif.Photo.FNumber                           Rational    1  F4
Exif.Photo.ExifVersion                       Undefined   4  1.10
Exif.Photo.DateTimeOriginal                  Ascii      20  2000:10:26 16:46:51
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.CompressedBitsPerPixel            Rational    1  (0/0)
Exif.Photo.ShutterSpeedValue                 SRational   1  1/32 s
Exif.Photo.ApertureValue                     Rational    1  F4
Exif.Photo.BrightnessValue                   SRational   1  1.5
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  F4
Exif.Photo.SubjectDistance                   Rational    1  Unknown
Exif.Photo.MeteringMode                      Short       1  Center weighted average
Exif.Photo.LightSource                       Short       1  Unknown
Exif.Photo.Flash                             Short       1  Fired
Exif.Photo.FocalLength                       Rational    1  4.4 mm
Exif.Photo.MakerNote                         Undefined  72  1 4 3 0 2 1 255 255 0 1 55 142 14 93 109 246 1 0 0 251 0 4 255 32 0 0 40 0 0 0 12 53 68 67 80 48 49 48 49 49 46 74 80 71 0 4 0 0 0 1 228 224 0 1 0 0 0 1 145 128 0 1 194 64 0 1 0 0 0 1 222 16
Exif.Thumbnail.ImageWidth                    Short       1  96
Exif.Thumbnail.ImageLength                   Short       1  72
Exif.Thumbnail.BitsPerSample                 Short       3  8 8 8
Exif.Thumbnail.Compression                   Short       1  Uncompressed
Exif.Thumbnail.PhotometricInterpretation     Short       1  RGB
Exif.Thumbnail.StripOffsets                  Short       1  832
Exif.Thumbnail.SamplesPerPixel               Short       1  3
Exif.Thumbnail.RowsPerStrip                  Short       1  72
Exif.Thumbnail.StripByteCounts               Short       1  20736
Exif.Thumbnail.XResolution                   Rational    1  72
Exif.Thumbnail.YResolution                   Rational    1  72
Exif.Thumbnail.ResolutionUnit                Short       1  inch
"""
    ]

    stderr = [""] * 6
    retval = [0] * 6

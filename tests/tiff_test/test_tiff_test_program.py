# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("{data_path}/mini9.tif")
class TestTiffTestProg(system_tests.Case):

    commands = ["{tiff-test} {data_path}/mini9_copy.tif"]

    stdout = [
        """Test 1: Writing empty Exif data without original binary data: ok.
Test 2: Writing empty Exif data with original binary data: ok.
Test 3: Wrote non-empty Exif data without original binary data:
Exif.Image.ExifTag                           0x8769 Long        1  26
Exif.Photo.DateTimeOriginal                  0x9003 Ascii      18  Yesterday at noon
MIME type:  image/tiff
Image size: 9 x 9
Before
Exif.Image.NewSubfileType                    0x00fe Long        1  0
Exif.Image.ImageWidth                        0x0100 Short       1  9
Exif.Image.ImageLength                       0x0101 Short       1  9
Exif.Image.BitsPerSample                     0x0102 Short       3  8 8 8
Exif.Image.Compression                       0x0103 Short       1  1
Exif.Image.PhotometricInterpretation         0x0106 Short       1  2
Exif.Image.DocumentName                      0x010d Ascii      24  /home/ahuggel/mini9.tif
Exif.Image.ImageDescription                  0x010e Ascii      18  Created with GIMP
Exif.Image.StripOffsets                      0x0111 Long        1  8
Exif.Image.Orientation                       0x0112 Short       1  1
Exif.Image.SamplesPerPixel                   0x0115 Short       1  3
Exif.Image.RowsPerStrip                      0x0116 Short       1  64
Exif.Image.StripByteCounts                   0x0117 Long        1  243
Exif.Image.XResolution                       0x011a Rational    1  1207959552/16777216
Exif.Image.YResolution                       0x011b Rational    1  1207959552/16777216
Exif.Image.PlanarConfiguration               0x011c Short       1  1
Exif.Image.ResolutionUnit                    0x0128 Short       1  2
======
After
Exif.Image.NewSubfileType                    0x00fe Long        1  0
Exif.Image.ImageWidth                        0x0100 Short       1  9
Exif.Image.ImageLength                       0x0101 Short       1  9
Exif.Image.BitsPerSample                     0x0102 Short       3  8 8 8
Exif.Image.Compression                       0x0103 Short       1  1
Exif.Image.PhotometricInterpretation         0x0106 Short       1  2
Exif.Image.DocumentName                      0x010d Ascii      24  /home/ahuggel/mini9.tif
Exif.Image.ImageDescription                  0x010e Ascii      18  Created with GIMP
Exif.Image.StripOffsets                      0x0111 Long        1  8
Exif.Image.Orientation                       0x0112 Short       1  1
Exif.Image.SamplesPerPixel                   0x0115 Short       1  3
Exif.Image.RowsPerStrip                      0x0116 Short       1  64
Exif.Image.StripByteCounts                   0x0117 Long        1  243
Exif.Image.XResolution                       0x011a Rational    1  1207959552/16777216
Exif.Image.YResolution                       0x011b Rational    1  1207959552/16777216
Exif.Image.PlanarConfiguration               0x011c Short       1  1
Exif.Image.ResolutionUnit                    0x0128 Short       1  2
Exif.Photo.DateTimeOriginal                  0x9003 Ascii      18  Yesterday at noon
"""
    ]
    stderr = [""]
    retval = [0]

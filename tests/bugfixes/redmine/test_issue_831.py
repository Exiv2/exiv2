# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, path


@CopyFiles("$data_path/mini9.tif")
class UseNonIntrusiveWriting(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/831"

    filename = path("$data_path/mini9_copy.tif")

    commands = [
        """$exiv2 -v -Qd -M"set Exif.Image.ImageDescription Just GIMP" $filename""",
        "$exiv2 -v -pa $filename"
    ]

    stdout = [
        """File 1/1: $filename
Set Exif.Image.ImageDescription "Just GIMP" (Ascii)
""",
        """File 1/1: $filename
Exif.Image.NewSubfileType                    Long        1  Primary image
Exif.Image.ImageWidth                        Short       1  9
Exif.Image.ImageLength                       Short       1  9
Exif.Image.BitsPerSample                     Short       3  8 8 8
Exif.Image.Compression                       Short       1  Uncompressed
Exif.Image.PhotometricInterpretation         Short       1  RGB
Exif.Image.DocumentName                      Ascii      24  /home/ahuggel/mini9.tif
Exif.Image.ImageDescription                  Ascii      10  Just GIMP
Exif.Image.StripOffsets                      Long        1  8
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.SamplesPerPixel                   Short       1  3
Exif.Image.RowsPerStrip                      Short       1  64
Exif.Image.StripByteCounts                   Long        1  243
Exif.Image.XResolution                       Rational    1  72
Exif.Image.YResolution                       Rational    1  72
Exif.Image.PlanarConfiguration               Short       1  1
Exif.Image.ResolutionUnit                    Short       1  inch
"""
    ]
    stderr = [
        """Info: Write strategy: Non-intrusive
""",
        """$filename: No IPTC data found in the file
$filename: No XMP data found in the file
"""
    ]
    retval = [0] * 2

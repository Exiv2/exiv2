# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, path


@CopyFiles("$data_path/exiv2-empty.jpg")
class FixMetadataDifferenceToExiftool(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/935"

    filename = path("$data_path/exiv2-empty_copy.jpg")

    commands = [
        """$exiv2 -u -v -M"set Exif.Photo.ExposureTime 605/10" $filename""",
        """$exiv2 -u -pv -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -pa -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -v -M"set Exif.Photo.ExposureTime 2/3" $filename""",
        """$exiv2 -u -pv -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -pa -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -v -M"set Exif.Photo.ExposureTime 0/0" $filename""",
        """$exiv2 -u -pv -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -pa -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -v -M"set Exif.Photo.ExposureTime 605/605" $filename""",
        """$exiv2 -u -pv -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -pa -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -v -M"set Exif.Photo.ExposureTime 100/1500" $filename""",
        """$exiv2 -u -pv -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -pa -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -v -M"set Exif.Photo.ExposureTime Ascii Test" $filename""",
        """$exiv2 -u -pv -gExif.Photo.ExposureTime $filename""",
        """$exiv2 -u -pa -gExif.Photo.ExposureTime $filename""",
    ]

    stdout = [
        """File 1/1: $filename
Set Exif.Photo.ExposureTime "605/10" (Rational)
""",
        """0x829a Photo        ExposureTime                Rational    1  605/10
""",
        """Exif.Photo.ExposureTime                      Rational    1  60.5 s
""",
        """File 1/1: $filename
Set Exif.Photo.ExposureTime "2/3" (Rational)
""",
        """0x829a Photo        ExposureTime                Rational    1  2/3
""",
        """Exif.Photo.ExposureTime                      Rational    1  0.666667 s
""",
        """File 1/1: $filename
Set Exif.Photo.ExposureTime "0/0" (Rational)
""",
        """0x829a Photo        ExposureTime                Rational    1  0/0
""",
        """Exif.Photo.ExposureTime                      Rational    1  (0/0)
""",
        """File 1/1: $filename
Set Exif.Photo.ExposureTime "605/605" (Rational)
""",
        """0x829a Photo        ExposureTime                Rational    1  605/605
""",
        """Exif.Photo.ExposureTime                      Rational    1  1 s
""",
        """File 1/1: $filename
Set Exif.Photo.ExposureTime "100/1500" (Rational)
""",
        """0x829a Photo        ExposureTime                Rational    1  100/1500
""",
        """Exif.Photo.ExposureTime                      Rational    1  1/15 s
""",
        """File 1/1: $filename
Set Exif.Photo.ExposureTime "Test" (Ascii)
""",
        """0x829a Photo        ExposureTime                Ascii       5  Test
""",
        """Exif.Photo.ExposureTime                      Ascii       5  (Test)
"""
    ]

    stderr = [""] * 18

    retval = [0] * 18

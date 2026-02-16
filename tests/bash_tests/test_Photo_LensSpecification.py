# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path

#####################################################################
# Standard cases (correct values)
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationCorrect(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 1000/10 1000/10 28/10 28/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 280/10 700/10 28/10 28/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 280/10 700/10 28/10 40/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 28/10 70/10 28/10 40/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "1000/10 1000/10 28/10 28/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  100mm F2.8
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "280/10 700/10 28/10 28/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  28-70mm F2.8
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "280/10 700/10 28/10 40/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  28-70mm F2.8-4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "28/10 70/10 28/10 40/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  2.8-7mm F2.8-4
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# empty lens specification (all zeros)
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationEmpty(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 0/0 0/0 0/0 0/0" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "0/0 0/0 0/0 0/0" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  n/a
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# Exif.Photo.LensSpecification filled with wrong type: Ascii
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationAscii(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification Ascii 1/10 2/10 3/10 4/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "1/10 2/10 3/10 4/10" (Ascii)
""",
    """Exif.Photo.LensSpecification                 Ascii      20  (1/10 2/10 3/10 4/10)
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# Exif.Photo.LensSpecification filled with wrong type: Byte (4 values)
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecification4Byte(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification Byte 1 2 3 4" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "1 2 3 4" (Byte)
""",
    """Exif.Photo.LensSpecification                 Byte        4  (1 2 3 4)
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# Exif.Photo.LensSpecification filled with wrong number of values
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecification5Rational(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 1/10 2/10 3/10 4/10 5/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "1/10 2/10 3/10 4/10 5/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    5  (1/10 2/10 3/10 4/10 5/10)
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# divisor is zero
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationZeroDevisor(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 1/0 2/10 3/10 4/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 1/10 2/0 3/10 4/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 1/10 2/10 3/0 4/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 1/10 2/10 3/10 4/0" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "1/0 2/10 3/10 4/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  (1/0 2/10 3/10 4/10)
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "1/10 2/0 3/10 4/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  (1/10 2/0 3/10 4/10)
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "1/10 2/10 3/0 4/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  (1/10 2/10 3/0 4/10)
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "1/10 2/10 3/10 4/0" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  (1/10 2/10 3/10 4/0)
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# dividend is zero
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationZeroDevidend(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 0/10 200/10 33/10 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 0/10 33/10 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 200/10 0/10 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 200/10 33/10 0/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "0/10 200/10 33/10 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  n/a-20mm F3.3-4.4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 0/10 33/10 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  10-n/a mm F3.3-4.4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 200/10 0/10 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  10-20mm F n/a-4.4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 200/10 33/10 0/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  10-20mm F3.3-n/a
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# dividend and divisor is zero
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationZeroDevidendDivisior(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 0/0 200/10 33/10 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 0/0 33/10 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 200/10 0/0 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 200/10 33/10 0/0" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "0/0 200/10 33/10 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  n/a-20mm F3.3-4.4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 0/0 33/10 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  10-n/a mm F3.3-4.4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 200/10 0/0 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  10-20mm F n/a-4.4
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 200/10 33/10 0/0" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  10-20mm F3.3-n/a
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

#####################################################################
# first value bigger than second
#####################################################################
@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class LensSpecificationFirstBiggerSecond(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-empty.jpg")

    commands = [
        """$exiv2 -u -v -M"add Exif.Photo.LensSpecification 2010/100 200/10 33/10 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename",
        """$exiv2 -u -v -M"set Exif.Photo.LensSpecification 100/10 200/10 441/100 44/10" $filename""",
        "$exiv2 -KExif.Photo.LensSpecification $filename"
    ]
    stdout = ["""File 1/1: $filename
Add Exif.Photo.LensSpecification "2010/100 200/10 33/10 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  (2010/100 200/10 33/10 44/10)
""",
    """File 1/1: $filename
Set Exif.Photo.LensSpecification "100/10 200/10 441/100 44/10" (Rational)
""",
    """Exif.Photo.LensSpecification                 Rational    4  (100/10 200/10 441/100 44/10)
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)


# -*- coding: utf-8 -*-

import sys
from system_tests import CaseMeta, CopyFiles, CopyTmpFiles, DeleteFiles, path

###########################################################
# rename with different formats
###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_02_Sep_2018.exv"
renformat = ":basename:_%d_%b_%Y"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename_dbY(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
	"""File 1/1: $infilename
Renaming file to $outfilename
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_2018-09-02-19-40.exv"
renformat = ":basename:_%Y-%m-%d-%H-%M"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename_YmdHM(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
	"""File 1/1: $infilename
Renaming file to $outfilename
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_NIKON D850_46.0 mm.exv"
renformat = ":basename:_:Exif.Image.Model:_:Exif.Photo.FocalLengthIn35mmFilm:"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename_ExifTags(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
	"""File 1/1: $infilename
Renaming file to $outfilename
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################

infile ="_DSC8437.exv"
if sys.platform == 'win32':
    outfile = "_DSC8437_a_b_c_d_e_f_g_h_i.exv"
else:
    outfile = "_DSC8437_a\\b_c_d*e?f<g>h|i.exv"

renformat = ":basename:_:Exif.Image.ImageDescription:"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename_ExifTagsInvalidChar(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        """$exiv2 -M"set Exif.Image.ImageDescription Ascii a\\b/c:d*e?f<g>h|i" $infilename""",
        "$exiv2 --grep Exif.Image.ImageDescription $infilename",
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
    "",
    """Exif.Image.ImageDescription                  Ascii      18  a\\b/c:d*e?f<g>h|i
""",
	"""File 1/1: $infilename
Renaming file to $outfilename
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# rename with keeping suffix
###########################################################

basename ="_DSC8437"
outfile = "02_Sep_2018.PANO.exv"
renformat = "%d_%b_%Y:basesuffix:"

@CopyTmpFiles("$data_path/_DSC8437.exv")
@DeleteFiles("$tmp_path/" + outfile)
class Rename_basesuffix(metaclass=CaseMeta):
    infilename1 = path("$tmp_path/" + basename + ".exv")
    infilename2 = path("$tmp_path/" + basename + ".PANO.exv")
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        # first command to prepare a file name with suffix
        "$exiv2 --verbose --rename :basename:.PANO "  + infilename1,
        "$exiv2 --verbose --rename " + renformat + " " + infilename2
    ]
    stdout = [
	"""File 1/1: $infilename1
Renaming file to $infilename2
""",
	"""File 1/1: $infilename2
Renaming file to $outfilename
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# rename error: tag is not included
###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_.exv"
renformat = ":basename:_:Exif.Image.ImageDescription:"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename_TagNotIncluded(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
	"""File 1/1: $infilename
Renaming file to $outfilename
"""
    ]
    stderr = ["""$infilename: Warning: Exif.Image.ImageDescription is not included.
"""]
    retval = [0] * len(commands)

###########################################################
# rename error: invalid tag name
###########################################################

infile ="_DSC8437.exv"
renformat = ":basename:_:Exif.Image.ImageDescript:"

@CopyTmpFiles("$data_path/" + infile)
class Rename_InvalidTagName(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    commands = [
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
	"""File 1/1: $infilename
"""
    ]
    stderr = ["""Exiv2 exception in rename action for file $infilename:
Invalid tag name or ifdId `ImageDescript', ifdId 1
"""]
    retval = [1] * len(commands)

###########################################################
# rename error: file contains no Exif data
###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_.exv"
renformat = ":basename:_:Exif.Image.ImageDescription:"

@CopyTmpFiles("$data_path/" + infile)
#@DeleteFiles("$tmp_path/" + outfile)
class Rename_NoExifData(metaclass=CaseMeta):
    infilename = path("$tmp_path/" + infile)
    outfilename = path("$tmp_path/" + outfile)
    commands = [
        "$exiv2 --delete a " + infilename,
        "$exiv2 --verbose --rename " + renformat + " " + infilename
    ]
    stdout = [
    "",
	"""File 1/1: $infilename
"""
    ]
    stderr = ["",
    """$infilename: No Exif data found in the file
"""]
    retval = [0, 253]


# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, DeleteFiles, path, FileDecoratorBase

###########################################################
# rename with different formats
###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_02_Sep_2018.exv"
renformat = ":basename:_%d_%b_%Y"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename1(metaclass=CaseMeta):
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
class Rename2(metaclass=CaseMeta):
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
class Rename3(metaclass=CaseMeta):
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
outfile = "_DSC8437_a_b_c_d_e_f_g_h_i.exv"
renformat = ":basename:_:Exif.Image.ImageDescription:"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class Rename4(metaclass=CaseMeta):
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
# rename error: tag is not included
###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_.exv"
renformat = ":basename:_:Exif.Image.ImageDescription:"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class RenameError1(metaclass=CaseMeta):
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
    stderr = ["""Warning: Exif.Image.ImageDescription is not included.
"""]
    retval = [0] * len(commands)

###########################################################
# rename error: unbalanced colon
###########################################################

infile ="_DSC8437.exv"
outfile = "_DSC8437_.exv"
renformat = ":basename:_Exif.Image.ImageDescription:"

@CopyTmpFiles("$data_path/" + infile)
@DeleteFiles("$tmp_path/" + outfile)
class RenameError2(metaclass=CaseMeta):
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
    stderr = ["""Warning: Exif.Image.ImageDescription is not included.
"""]
    retval = [0] * len(commands)


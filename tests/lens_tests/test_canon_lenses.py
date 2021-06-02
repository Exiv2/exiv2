# -*- coding: utf-8 -*-

import re
import os
import system_tests
import math
from lens_tests.utils import extract_lenses_from_cpp, make_test_cases

# get directory of the current file
file_dir = os.path.dirname(os.path.realpath(__file__))
# to get the canon maker note cpp file that contains list of all supported lenses
canon_lens_file = os.path.abspath(os.path.join(file_dir, "./../../src/canonmn_int.cpp"))
# tell the below function what the start of the lens array looks like
startpattern = "constexpr TagDetails canonCsLensType[] = {"
# use utils function to extract all lenses
lenses = extract_lenses_from_cpp(canon_lens_file, startpattern)
# use utils function to define test case data
test_cases = make_test_cases(lenses)

# see https://github.com/exiftool/exiftool/blob/master/lib/Image/ExifTool/Canon.pm#L9678
def aperture_to_raw_exif(aperture):
    # for apertures < 1 the below is negative
    num = math.log(aperture) * 2 / math.log(2)

    # temporarily make the number positive
    if num < 0:
        num = -num
        sign = -1
    else:
        sign = 1

    val = int(num)
    frac = num - val

    if abs(frac - 0.33) < 0.05:
        frac = 0x0C
    elif abs(frac - 0.67) < 0.05:
        frac = 0x14
    else:
        frac = int(frac * 0x20 + 0.5)

    return sign * (val * 0x20 + frac)


for lens_tc in test_cases:

    testname = lens_tc["id"] + "_" + lens_tc["desc"]

    globals()[testname] = system_tests.CaseMeta(
        "canon_lenses." + testname,
        tuple(),
        {
            "filename": "$data_path/template.exv",
            "commands": [
                '$exiv2 -M"set Exif.CanonCs.LensType $lens_id" -M"set Exif.CanonCs.Lens $focal_length_max $focal_length_min 1" -M"set Exif.CanonCs.MaxAperture $aperture_max" $filename && $exiv2 -pa -K Exif.CanonCs.LensType $filename'
            ],
            "stderr": [""],
            "stdout": ["Exif.CanonCs.LensType                        Short       1  $lens_description\n"],
            "retval": [0],
            "lens_id": lens_tc["id"],
            "lens_description": lens_tc["target"],
            "aperture_max": aperture_to_raw_exif(lens_tc["aperture_max_short"]),
            "focal_length_min": int(lens_tc["focal_length_min"] * lens_tc["tc"]),
            "focal_length_max": int(lens_tc["focal_length_max"] * lens_tc["tc"]),
        },
    )

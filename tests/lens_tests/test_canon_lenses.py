# -*- coding: utf-8 -*-

import re
import os
import system_tests
from lens_tests.utils import extract_lenses_from_cpp

# get directory of the current file
file_dir = os.path.dirname(os.path.realpath(__file__))
# to get the canon maker note cpp file that contains list of all supported lenses
canon_lens_file = os.path.abspath(os.path.join(file_dir, "./../../src/canonmn_int.cpp"))
# tell the below function what the start of the lens array looks like
startpattern = "constexpr TagDetails canonCsLensType[] = {"
# use utils function to extract all lenses
lenses = extract_lenses_from_cpp(canon_lens_file, startpattern)

apertures = (
    "1.0",
    "1.1",
    "1.2",
    "1.4",
    "1.6",
    "1.8",
    "2",
    "2.2",
    "2.5",
    "2.8",
    "3.2",
    "3.5",
    "4",
    "4.5",
    "5",
    "5.6",
    "6.3",
    "7.1",
    "8",
    "9",
    "10",
    "11",
    "13",
    "14",
    "16",
    "18",
    "20",
    "22",
    "25",
    "29",
    "32",
    "36",
    "40",
    "45",
)

fractions = {0: 0, 1: 12, 2: 20}
aperture_map = {value: (index // 3) * 32 + fractions[index % 3] for index, value in enumerate(apertures)}
aperture_map["3.8"] = "32000"  # aperture value cannot be represented in metadata, ignore it by using invalid value

for (lens_id, lens_desc, meta) in lenses[:5]:

    tc = float(meta["tc"] or 1)

    testname = lens_id + "_" + lens_desc

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
            "lens_id": lens_id,
            "lens_description": lens_desc,
            "aperture_max": aperture_map[meta["aperture_min"] or meta["aperture_max"]],
            "focal_length_min": int(int(meta["focal_length_min"] or meta["focal_length_max"]) * tc),
            "focal_length_max": int(int(meta["focal_length_max"]) * tc),
        },
    )

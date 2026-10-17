# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, path


@CopyFiles("$data_path/issue_3414.exv")
class UnicodeCommentEncodedAsUtf8(metaclass=CaseMeta):
    """
    Exif 3.0 specifies UTF-8 for the "UNICODE" character code, older writers use UCS-2.
    issue_3414.exv (II byte order) contains:
      - UserComment: "UNICODE\\0" + UTF-8 "été ✓ 日本" (no BOM), as written by an Exif 3.0 writer
      - GPSProcessingMethod: "UNICODE\\0" + UCS-2LE "été" (no BOM), as written by exiv2 and ExifTool
    Both must be read correctly, and exiv2 must keep writing UCS-2.
    """

    url = "https://github.com/Exiv2/exiv2/issues/3414"

    filename = path("$data_path/issue_3414_copy.exv")
    # Use the """ string syntax for -M"set Bla.Dee.Bla" commands to ensure they run on Windows!
    commands = [
        "$exiv2 -pt -g UserComment -g GPSProcessingMethod $filename",
        """$exiv2 -M"set Exif.Photo.UserComment charset=Unicode \\u00e9t\\u00e9" $filename""",
        "$exiv2 -PEnh -g UserComment $filename",
        "$exiv2 -pt -g UserComment $filename",
    ]
    stdout = [
        """Exif.Photo.UserComment                       Undefined  24  charset=Unicode été ✓ 日本
Exif.GPSInfo.GPSProcessingMethod             Undefined  14  charset=Unicode été
""",
        "",
        """UserComment                
  0000  55 4e 49 43 4f 44 45 00 e9 00 74 00 e9 00        UNICODE...t...

""",
        """Exif.Photo.UserComment                       Undefined  14  charset=Unicode été
""",
    ]
    stderr = [""] * 4
    retval = [0] * 4

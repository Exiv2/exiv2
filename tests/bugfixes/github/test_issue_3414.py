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


@CopyFiles("$data_path/issue_3414.exv")
class UnicodeCommentUcs2CyrillicLittleEndian(metaclass=CaseMeta):
    """
    A UCS-2LE comment in U+0400-U+047F ("Привет") has no NUL byte and only bytes
    below 0x80, so it is also well-formed UTF-8. It must still be read as UCS-2.
    """

    url = "https://github.com/Exiv2/exiv2/issues/3414"

    filename = path("$data_path/issue_3414_copy.exv")
    # Use the """ string syntax for -M"set Bla.Dee.Bla" commands to ensure they run on Windows!
    commands = [
        """$exiv2 -M"set Exif.Photo.UserComment charset=Unicode \\u041f\\u0440\\u0438\\u0432\\u0435\\u0442" $filename""",
        "$exiv2 -pt -g UserComment $filename",
    ]
    stdout = [
        "",
        """Exif.Photo.UserComment                       Undefined  20  charset=Unicode Привет
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2


@CopyFiles("$data_path/issue_3414.exv")
class UnicodeCommentUcs2ArabicLittleEndian(metaclass=CaseMeta):
    """A UCS-2LE Arabic comment ("مرحبا") must not be mistaken for UTF-8 either."""

    url = "https://github.com/Exiv2/exiv2/issues/3414"

    filename = path("$data_path/issue_3414_copy.exv")
    commands = [
        """$exiv2 -M"set Exif.Photo.UserComment charset=Unicode \\u0645\\u0631\\u062d\\u0628\\u0627" $filename""",
        "$exiv2 -pt -g UserComment $filename",
    ]
    stdout = [
        "",
        """Exif.Photo.UserComment                       Undefined  18  charset=Unicode مرحبا
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2


@CopyFiles("$data_path/issue_3414_mm.exv")
class UnicodeCommentUcs2CyrillicBigEndian(metaclass=CaseMeta):
    """
    Same as UnicodeCommentUcs2CyrillicLittleEndian, but the MM file makes exiv2
    store the comment as UCS-2BE.
    """

    url = "https://github.com/Exiv2/exiv2/issues/3414"

    filename = path("$data_path/issue_3414_mm_copy.exv")
    commands = [
        """$exiv2 -M"set Exif.Photo.UserComment charset=Unicode \\u041f\\u0440\\u0438\\u0432\\u0435\\u0442" $filename""",
        "$exiv2 -pt -g UserComment $filename",
    ]
    stdout = [
        "",
        """Exif.Photo.UserComment                       Undefined  20  charset=Unicode Привет
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2


@CopyFiles("$data_path/issue_3414.exv")
class GpsAreaInformationUcs2LittleEndian(metaclass=CaseMeta):
    """GPSAreaInformation uses the same character code scheme as UserComment."""

    url = "https://github.com/Exiv2/exiv2/issues/3414"

    filename = path("$data_path/issue_3414_copy.exv")
    commands = [
        """$exiv2 -M"set Exif.GPSInfo.GPSAreaInformation charset=Unicode \\u041c\\u043e\\u0441\\u043a\\u0432\\u0430" $filename""",
        "$exiv2 -pt -g GPSAreaInformation $filename",
    ]
    stdout = [
        "",
        """Exif.GPSInfo.GPSAreaInformation              Undefined  20  charset=Unicode Москва
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2


@CopyFiles("$data_path/issue_3414_mm.exv")
class Utf8CommentEdgeCasesInBigEndianFile(metaclass=CaseMeta):
    """
    issue_3414_mm.exv (MM byte order) contains:
      - UserComment: "UNICODE\\0" + UTF-8 BOM + UTF-8 "Привет"
      - GPSAreaInformation: "UNICODE\\0" + UTF-8 "مرحبا" (no BOM)
      - GPSProcessingMethod: "UNICODE\\0" + malformed UTF-8 C3 28 C3 28,
        which falls back to UCS-2BE and reads as U+C328 U+C328
    """

    url = "https://github.com/Exiv2/exiv2/issues/3414"

    filename = path("$data_path/issue_3414_mm_copy.exv")
    commands = [
        "$exiv2 -pt -g UserComment $filename",
        "$exiv2 -pt -g GPSAreaInformation $filename",
        "$exiv2 -pt -g GPSProcessingMethod $filename",
    ]
    stdout = [
        """Exif.Photo.UserComment                       Undefined  23  charset=Unicode Привет
""",
        """Exif.GPSInfo.GPSAreaInformation              Undefined  18  charset=Unicode مرحبا
""",
        """Exif.GPSInfo.GPSProcessingMethod             Undefined  12  charset=Unicode 쌨쌨
""",
    ]
    stderr = [""] * 3
    retval = [0] * 3

# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path

# The rest of the summary after writing Exif. The fields are empty, so each
# line ends in ": ".
EMPTY_FIELDS = [
    "Camera make     ",
    "Camera model    ",
    "Image timestamp ",
    "File number     ",
    "Exposure time   ",
    "Aperture        ",
    "Exposure bias   ",
    "Flash           ",
    "Flash bias      ",
    "Focal length    ",
    "Subject distance",
    "ISO speed       ",
    "Exposure mode   ",
    "Metering mode   ",
    "Macro mode      ",
    "Image quality   ",
    "White balance   ",
    "Copyright       ",
    "Exif comment    ",
]
SUMMARY_TAIL = "Thumbnail       : None\n" + "".join(field + ": \n" for field in EMPTY_FIELDS) + "\n"


@CopyTmpFiles(
    "$data_path/webp_canvas_size_vp8.webp",
    "$data_path/webp_canvas_size_vp8l.webp",
    "$data_path/webp_canvas_size_anmf.webp",
)
class WebPCanvasSize(metaclass=CaseMeta):
    """
    Read the image size from WebP files whose first image chunk is VP8,
    VP8L or ANMF (no VP8X chunk). Then write Exif, which makes exiv2
    insert a VP8X chunk with the same size, and read the size back.
    """

    vp8 = path("$tmp_path/webp_canvas_size_vp8.webp")
    vp8l = path("$tmp_path/webp_canvas_size_vp8l.webp")
    anmf = path("$tmp_path/webp_canvas_size_anmf.webp")

    commands = [
        "$exiv2 $vp8",
        "$exiv2 $vp8l",
        "$exiv2 $anmf",
        """$exiv2 -M"set Exif.Image.Artist Exiv2" $vp8""",
        """$exiv2 -M"set Exif.Image.Artist Exiv2" $vp8l""",
        """$exiv2 -M"set Exif.Image.Artist Exiv2" $anmf""",
        "$exiv2 $vp8",
        "$exiv2 $vp8l",
        "$exiv2 $anmf",
    ]

    stdout = [
        """File name       : $vp8
File size       : 36 Bytes
MIME type       : image/webp
Image size      : 640 x 480
""",
        """File name       : $vp8l
File size       : 30 Bytes
MIME type       : image/webp
Image size      : 16384 x 300
""",
        """File name       : $anmf
File size       : 54 Bytes
MIME type       : image/webp
Image size      : 70000 x 2
""",
        "",
        "",
        "",
        """File name       : $vp8
File size       : 94 Bytes
MIME type       : image/webp
Image size      : 640 x 480
"""
        + SUMMARY_TAIL,
        """File name       : $vp8l
File size       : 88 Bytes
MIME type       : image/webp
Image size      : 16384 x 300
"""
        + SUMMARY_TAIL,
        """File name       : $anmf
File size       : 112 Bytes
MIME type       : image/webp
Image size      : 70000 x 2
"""
        + SUMMARY_TAIL,
    ]

    stderr = [
        "$vp8: No Exif data found in the file\n",
        "$vp8l: No Exif data found in the file\n",
        "$anmf: No Exif data found in the file\n",
    ] + [""] * 6

    retval = [253] * 3 + [0] * 6

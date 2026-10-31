# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, path


@CopyFiles("$data_path/PanasonicDMC-ZS7.exv")
class PanasonicMetadataInterpretation(metaclass=CaseMeta):
    """Keep real Panasonic parent blocks readable through metadata growth."""

    filename = path("$data_path/PanasonicDMC-ZS7_copy.exv")
    commands = [
        "$exiv2 -Pt -K Exif.Panasonic.FaceDetInfo -K Exif.Panasonic.FaceRecInfo $filename",
        # Add newer scalar values using their original encoded TIFF types.
        """$exiv2 -M"set Exif.Panasonic.PhotoStyle Short 17" -M"set Exif.Panasonic.FocusBracket Short 65535" $filename""",
        "$exiv2 -Pt -K Exif.Panasonic.FaceDetInfo -K Exif.Panasonic.FaceRecInfo -K Exif.Panasonic.PhotoStyle -K Exif.Panasonic.FocusBracket $filename",
        "$exiv2 -Pv -K Exif.Panasonic.FocusBracket $filename",
        # An unrelated edit must retain both parent blocks and newer fields.
        """$exiv2 -M"set Exif.Image.Artist Ascii metadata growth" $filename""",
        "$exiv2 -Pt -K Exif.Panasonic.FaceDetInfo -K Exif.Panasonic.FaceRecInfo -K Exif.Panasonic.PhotoStyle -K Exif.Panasonic.FocusBracket $filename",
    ]
    stdout = [
        "Stored face positions: 0\nFaces recognized: 0\n",
        "",
        "Stored face positions: 0\nFaces recognized: 0\nV-Log\n-1\n",
        "65535\n",
        "",
        "Stored face positions: 0\nFaces recognized: 0\nV-Log\n-1\n",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

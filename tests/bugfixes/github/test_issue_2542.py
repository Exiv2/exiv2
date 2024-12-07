from system_tests import CaseMeta, path


class OMSystemMakerNoteAlias(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2542
    """

    url = "https://github.com/Exiv2/exiv2/issues/2542"

    filename = path("$data_path/poc_2542.exv")
    commands = ["$exiv2 -q -K Exif.Olympus2.CameraID $filename"]
    stdout = [
        """Exif.Olympus2.CameraID                       Undefined  32  OLYMPUS DIGITAL CAMERA         
"""
    ]
    stderr = [""]
    retval = [0]

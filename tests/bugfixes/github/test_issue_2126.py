from system_tests import CaseMeta, path


class OMSystemMakerNote(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2126
    """

    url = "https://github.com/Exiv2/exiv2/issues/2126"

    filename = path("$data_path/test_issue_2126.exv")
    commands = ["$exiv2 -q -K Exif.Olympus2.CameraID $filename"]
    stdout = [
        """Exif.Olympus2.CameraID                       Undefined  32  OM SYSTEM CAMERA               
"""
    ]
    stderr = [""]
    retval = [0]

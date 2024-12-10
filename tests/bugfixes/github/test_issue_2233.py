from system_tests import CaseMeta, path


class TestJXLBoxEndXML(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2233
    """

    url = "https://github.com/Exiv2/exiv2/issues/2233"

    filename = path("$data_path/issue_2233_poc1.jxl")
    commands = ["$exiv2 -pS $filename"]
    stdout = [
        """Exiv2::BmffImage::boxHandler: JXL         0->12 
Exiv2::BmffImage::boxHandler: ftyp       12->20 brand: jxl 
Exiv2::BmffImage::boxHandler: jxlc       32->15060 
Exiv2::BmffImage::boxHandler: Exif    15092->258 
Exiv2::BmffImage::boxHandler: xml     15350->3699 
"""
    ]
    stderr = [""]
    retval = [0]


class TestJXLBoxEndExif(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2233
    """

    url = "https://github.com/Exiv2/exiv2/issues/2233"

    filename = path("$data_path/issue_2233_poc2.jxl")
    commands = ["$exiv2 -pS $filename"]
    stdout = [
        """Exiv2::BmffImage::boxHandler: JXL         0->12 
Exiv2::BmffImage::boxHandler: ftyp       12->20 brand: jxl 
Exiv2::BmffImage::boxHandler: jxlc       32->15060 
Exiv2::BmffImage::boxHandler: xml     15092->3699 
Exiv2::BmffImage::boxHandler: Exif    18791->258 
"""
    ]
    stderr = [""]
    retval = [0]

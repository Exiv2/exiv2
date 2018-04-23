# -*- coding: utf-8 -*-

import system_tests


class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the first bug described in:
    https://github.com/Exiv2/exiv2/issues/246
    """
    url = "https://github.com/Exiv2/exiv2/issues/246"

    filename = "$data_path/1-string-format.jpg"
    commands = ["$exiv2 -pS " + filename]
    stdout = [
        """STRUCTURE OF JPEG FILE: """ + filename + """
 address | marker       |  length | data
       0 | 0xffd8 SOI  
       2 | 0xffe1 APP1  |      60 | Exif..II*.....0.i...........0000
"""]


    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerNoImageInInputData
"""]
    retval = [1]

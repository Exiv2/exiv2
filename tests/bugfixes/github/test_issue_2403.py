# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles


@CopyTmpFiles("$data_path/issue_2403_poc.exv")
class checkIconvSegFault(metaclass=CaseMeta):
    url = """"""
    description = """Test the fixcom action in the exiv2 app"""

    filename = """$tmp_path/issue_2403_poc.exv"""

    commands = [
        """$exiv2 --verbose --log e --encode made_up_encoding fixcom $filename""",
        """$exiv2 --verbose --keep --encode UCS-2LE fixcom $filename""",
    ]
    retval = [1, 0]

    stdout = [
        """File 1/1: $filename
""",
        """File 1/1: $filename
Setting Exif UNICODE user comment to "Test"
""",
    ]

    stderr = [
        """Exiv2 exception in fixcom action for file $filename:
Cannot convert text encoding from 'made_up_encoding' to 'UTF-8'
""",
        """""",
    ]

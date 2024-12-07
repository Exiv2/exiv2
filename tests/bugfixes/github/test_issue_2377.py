# -*- coding: utf-8 -*-

from system_tests import CaseMeta


class issue_2377_QuickTimeVideo_userDataDecoder_buffer_overflow(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/2377"
    filename = "$data_path/issue_2377_poc.mp4"
    commands = ["$exiv2 $filename"]
    retval = [253]
    stderr = [
        """$filename: No Exif data found in the file
"""
    ]
    stdout = [
        """File name       : $filename
File size       : 225 Bytes
MIME type       : video/quicktime
Image size      : 0 x 0
"""
    ]

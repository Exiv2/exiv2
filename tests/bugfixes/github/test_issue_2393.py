# -*- coding: utf-8 -*-

from system_tests import CaseMeta


class issue_2393_QuickTimeVideo_multipleEntriesDecoder_long_running(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/2393"
    filename = "$data_path/issue_2393_poc.mp4"
    commands = ["$exiv2 $filename"]
    retval = [253]
    stderr = [
        """$filename: No Exif data found in the file
"""
    ]
    stdout = [
        """File name       : $filename
File size       : 319 Bytes
MIME type       : video/quicktime
Image size      : 0 x 0
"""
    ]

# -*- coding: utf-8 -*-

import system_tests
class issue_1522_exif_asan(metaclass=system_tests.CaseMeta):
    url      = "https://github.com/Exiv2/exiv2/issues/1522"
    filename = "$data_path/poc_1522.jp2"
    commands = ["$exiv2     $filename"
               ,"$exiv2 -pS $filename"
               ]
    retval   = [ 253,1 ] 
    stderr   = [ """Warning: Failed to decode Exif metadata.
$filename: No Exif data found in the file
""","""$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""] 
    stdout   = ["""File name       : $filename
File size       : 268 Bytes
MIME type       : image/jp2
Image size      : 0 x 0
""","""STRUCTURE OF JPEG2000 FILE: $filename
 address |   length | box       | data
       0 |       12 | jP        | 
      12 |       25 | uuid      | Exif: .
"""
]

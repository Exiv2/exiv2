# -*- coding: utf-8 -*-

import system_tests
import unittest

# test needs system_tests.BT.vv['enable_bmff']=1
bSkip=system_tests.BT.verbose_version().get('enable_bmff')!='1'
if bSkip:
    raise unittest.SkipTest('*** requires enable_bmff=1 ***')

class issue_1503_jxl_bmff(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issue/1503"
    filename = "$data_path/Reagan.jxl"
    commands = ["$exiv2  -pS       $filename"
               ,"$exiv2  -g Date/i $filename"
               ,"$exiv2            $filename"
               ]
    retval = [ 0  ] * len(commands)
    stderr = [ "" ] * len(commands)
    stdin  = [ "" ] * len(commands)
    stdout = ["""Exiv2::BmffImage::boxHandler: JXL         0->12 
Exiv2::BmffImage::boxHandler: ftyp       12->20 brand: jxl 
Exiv2::BmffImage::boxHandler: Exif       32->5722 
Exiv2::BmffImage::boxHandler: xml      5754->5306 
Exiv2::BmffImage::boxHandler: jbrd    11060->1707 
Exiv2::BmffImage::boxHandler: jxlc    12767->20125 
""","""Exif.Image.DateTime                          Ascii      20  2016:09:13 11:58:16
Exif.Photo.DateTimeOriginal                  Ascii      20  2004:06:21 23:37:53
Exif.Photo.DateTimeDigitized                 Ascii      20  2004:06:21 23:37:53
Xmp.xmp.ModifyDate                           XmpText    25  2016-09-13T11:58:16+01:00
Xmp.xmp.CreateDate                           XmpText    25  2004-06-21T23:37:53+01:00
Xmp.xmp.MetadataDate                         XmpText    25  2016-09-13T11:58:16+01:00
Xmp.photoshop.DateCreated                    XmpText    10  2004-06-21
""","""File name       : $filename
File size       : 32892 Bytes
MIME type       : image/generic
Image size      : 200 x 130
Thumbnail       : image/jpeg, 4196 Bytes
Camera make     : NIKON CORPORATION
Camera model    : NIKON D1X
Image timestamp : 2004:06:21 23:37:53
File number     : 
Exposure time   : 1/125 s
Aperture        : F5
Exposure bias   : +1/3 EV
Flash           : No flash
Flash bias      : 
Focal length    : 42.0 mm
Subject distance: 
ISO speed       : 
Exposure mode   : Manual
Metering mode   : Center weighted average
Macro mode      : 
Image quality   : 
White balance   : Manual
Copyright       : 
Exif comment    : 

"""
]

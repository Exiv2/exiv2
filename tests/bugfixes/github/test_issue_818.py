# -*- coding: utf-8 -*-

import system_tests

class TestDefaultCommandGrep(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/818"

    filename = system_tests.path("$data_path/Reagan.jpg")
    commands = [
        "$exiv2     -g Date $filename",
        "$exiv2 -px -g Date $filename",
    ]
    retval = [0, 0]
    stderr =   [""] * 2
    retval =    [0] * 2

    stdout = [
        """Exif.Image.DateTime                          Ascii      20  2016:09:13 11:58:16
Exif.Photo.DateTimeOriginal                  Ascii      20  2004:06:21 23:37:53
Exif.Photo.DateTimeDigitized                 Ascii      20  2004:06:21 23:37:53
Iptc.Application2.DateCreated                Date        8  2004-06-21
Iptc.Application2.DigitizationDate           Date        8  2004-06-21
Xmp.xmp.ModifyDate                           XmpText    25  2016-09-13T11:58:16+01:00
Xmp.xmp.CreateDate                           XmpText    25  2004-06-21T23:37:53+01:00
Xmp.xmp.MetadataDate                         XmpText    25  2016-09-13T11:58:16+01:00
Xmp.photoshop.DateCreated                    XmpText    10  2004-06-21\n""",
        """Xmp.xmp.ModifyDate                           XmpText    25  2016-09-13T11:58:16+01:00
Xmp.xmp.CreateDate                           XmpText    25  2004-06-21T23:37:53+01:00
Xmp.xmp.MetadataDate                         XmpText    25  2016-09-13T11:58:16+01:00
Xmp.photoshop.DateCreated                    XmpText    10  2004-06-21\n"""
    ]

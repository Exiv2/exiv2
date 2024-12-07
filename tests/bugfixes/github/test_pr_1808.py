# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


# Full support for SonyMisc3c tags using this model
class SonyMisc3cSupportedTest(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-SonyDSC-HX60V.exv")
    commands = ["$exiv2 -pa --grep SonySInfo1 $filename"]

    stdout = [
        """Exif.SonySInfo1.SonyDateTime                 Ascii      20  2014:01:01 00:52:22
Exif.SonySInfo1.SonyImageHeight              Short       1  3888
Exif.SonySInfo1.SonyImageWidth               Short       1  5184
Exif.SonySInfo1.FacesDetected                Short       1  0
Exif.SonySInfo1.MetaVersion                  Ascii      16  DC7303320222000
"""
    ]
    stderr = [""]
    retval = [0]

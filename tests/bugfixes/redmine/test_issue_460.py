# -*- coding: utf-8 -*-

import system_tests


class EncodingTest(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/460"

    filename = "$data_path/exiv2-bug460.jpg"

    commands = ["$exiv2 -u -pt $filename"]

    stdout = ["""Exif.Image.XPTitle                           Byte       40  Titel und ähnliches
Exif.Image.XPComment                         Byte       46  And a standard comment
Exif.Image.XPAuthor                          Byte       44  Українська (Russian?)
Exif.Image.XPKeywords                        Byte       30  עברית (Hebrew)
Exif.Image.XPSubject                         Byte       32  తెలుగు (Telugu)
"""]
    stderr = [""]
    retval = [0]

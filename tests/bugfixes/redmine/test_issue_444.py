# -*- coding: utf-8 -*-

import system_tests


class PanicOnMisinformedIPTC(metaclass=system_tests.CaseMeta):

    encodings = ['latin-1']

    url = "http://dev.exiv2.org/issues/444"

    filename = "$data_path/exiv2-bug444.jpg"
    commands = ["$exiv2 -u -pi $filename"]
    stdout = ["""Iptc.Envelope.CharacterSet                   String      4  \x00Ä\x1c\x02
Iptc.Application2.Urgency                    String      2  GT
Iptc.Application2.SuppCategory               String      8  portrait
Iptc.Application2.Caption                    String     18  Witty caption here
Iptc.Application2.Keywords                   String      3  ize
Iptc.Application2.0x007b                     String      4  n123
"""]
    stderr = [""]
    retval = [0]

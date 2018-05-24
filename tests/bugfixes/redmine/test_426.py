# -*- coding: utf-8 -*-

import system_tests


class ReadIPTfromJPEG(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/426"

    filename = "$data_path/exiv2-bug426.jpg"
    commands = ["$exiv2 -u -pi $filename"]
    stdout = ["""Iptc.Application2.RecordVersion              Short       1  2
Iptc.Application2.FixtureId                  String     22  2005 Oregon Coast Tour
Iptc.Application2.Byline                     String     14  Julie V. Early
Iptc.Application2.City                       String     21  159 km SW of Portland
Iptc.Application2.ProvinceState              String      6  Oregon
Iptc.Application2.CountryName                String     13  United States
Iptc.Application2.Contact                    String     16  Jeffrey J. Early
"""]
    stderr = [""]
    retval = [0]

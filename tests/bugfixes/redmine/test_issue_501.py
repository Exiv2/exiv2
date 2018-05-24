# -*- coding: utf-8 -*-

import system_tests


class IptcFieldParsingTooStrict(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/501"

    filename = "$data_path/exiv2-bug501.jpg"

    commands = ["$exiv2 -u -pi $filename"]

    encodings = ['windows-1252']

    stdout = ["""Iptc.Application2.RecordVersion              Short       1  2
Iptc.Application2.Caption                    String     15  Porträtt på Eva
Iptc.Application2.Writer                     String     10  Elsa Nordh
Iptc.Application2.Byline                     String     10  Elsa Nordh
Iptc.Application2.ObjectName                 String      7  Hundbus
Iptc.Application2.DateCreated                Date        8  2006-10-31
Iptc.Application2.City                       String      8  Göteborg
Iptc.Application2.ProvinceState              String     15  Västra Götaland
Iptc.Application2.CountryName                String      6  Sweden
Iptc.Application2.Keywords                   String      3  Eva
Iptc.Application2.Keywords                   String      7  Gallery
Iptc.Application2.Keywords                   String     21  Framkalla jul06 mamma
Iptc.Application2.Copyright                  String     19  http://nordharna.se
Iptc.Application2.ExpirationTime             String      2  SE
Iptc.Application2.SubLocation                String      9  KÃ¥lltorp
Iptc.Application2.0x00d1                     String      9  GÃ¶teborg
Iptc.Application2.0x00d2                     String     17  VÃ¤stra GÃ¶taland
Iptc.Application2.0x00d3                     String      6  416 81
Iptc.Application2.0x00d4                     String      6  Sweden
Iptc.Application2.0x00d5                     String     19  +46-(0)736-21 50 74
Iptc.Application2.0x00d6                     String     17  foto@nordharna.se
Iptc.Application2.0x00d7                     String     27  http://gallery.nordharna.se
Iptc.Application2.0x00e3                     String     27  http://gallery.nordharna.se
Iptc.Application2.0x00f2                     String      1  4
"""]
    stderr = ["""Warning: Unsupported time format
"""]
    retval = [0]

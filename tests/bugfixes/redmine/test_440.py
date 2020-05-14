# -*- coding: utf-8 -*-

import system_tests


class IPTCtimeWithoutTimeZone(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/440"

    filename = "$data_path/exiv2-bug440.jpg"
    commands = ["$exiv2 -u -b -pi $filename"]
    stdout = ["""Iptc.Application2.RecordVersion              Short       1  1
Iptc.Application2.ObjectName                 String     16  WHITE SOX TIGERS
Iptc.Application2.Category                   String      1  S
Iptc.Application2.SuppCategory               String      3  BBA
Iptc.Application2.DateCreated                Date        8  2005-09-29
Iptc.Application2.TimeCreated                Time       11  16:51:00+00:00
Iptc.Application2.Byline                     String     11  PAUL SANCYA
Iptc.Application2.BylineTitle                String      3  STF
Iptc.Application2.City                       String      7  DETROIT
Iptc.Application2.ProvinceState              String      2  MI
Iptc.Application2.CountryName                String      3  USA
Iptc.Application2.TransmissionReference      String      6  DTS112
Iptc.Application2.Headline                   String     14  GARCIA GARLAND
Iptc.Application2.Credit                     String      2  AP
Iptc.Application2.Source                     String      2  AP
Iptc.Application2.Caption                    String    282  Chicago White Sox pitcher John Garland, left, pours champagne on Freddy Garcia in the locker room after their 4-2 win against the Detroit Tigers in Detroit, Thursday, Sept. 29, 2005. With the win the White Sox clinched their first AL Central title since 2000. (AP Photo/Paul Sancya)
Iptc.Application2.Writer                     String     15  DD PCS DD**NY**
"""]
    stderr = [""]
    retval = [0]

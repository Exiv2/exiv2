# -*- coding: utf-8 -*-

import system_tests


class CheckTimeZones(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1231"
    filenames = ["$data_path/exiv2-bug1231a.jpg",
                 "$data_path/exiv2-bug1231b.jpg"]

    commands = [
        "$exiv2 -pa --grep CanonTi/i " + filenames[0],
        "$exiv2 -pa --grep CanonTi/i " + filenames[1]
    ]

    stdout = [
        """Exif.CanonTi.TimeZone                        SLong       1  60
Exif.CanonTi.TimeZoneCity                    SLong       1  n/a
Exif.CanonTi.DaylightSavings                 SLong       1  0
""",
        """Exif.CanonTi.TimeZone                        SLong       1  120
Exif.CanonTi.TimeZoneCity                    SLong       1  n/a
Exif.CanonTi.DaylightSavings                 SLong       1  60
""",
    ]

    stderr = [""] * len(stdout)
    retval = [0] * len(stdout)

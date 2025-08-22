# -*- coding: utf-8 -*-

import system_tests


class PanasonicManometerTag(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/825"

    filenames = [
        f"$data_path/exiv2-bug825{char!s}.exv"
        for char in ["a", "b"]
    ]

    commands = []
    for filename in filenames:
        commands.append(f"$exiv2 -pv --grep mano/i {filename}")
        commands.append(f"$exiv2 -pa --grep mano/i {filename}")

    stdout = [
        """0x0086 Panasonic    ManometerPressure           Short       1  65535
""",
        """Exif.Panasonic.ManometerPressure             Short       1  infinite
""",
        """0x0086 Panasonic    ManometerPressure           Short       1  1007
""",
        """Exif.Panasonic.ManometerPressure             Short       1  1007 hPa
"""
    ]
    stderr = [""] * 4
    retval = [0] * 4

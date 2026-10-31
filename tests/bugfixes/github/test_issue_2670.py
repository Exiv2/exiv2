# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path


@CopyTmpFiles("$data_path/exiv2-SonyDSC-HX60V.exv")
class SonyMisc3cStringExport(metaclass=CaseMeta):
    """Recreating an unknown field through a string must preserve the Sony array."""

    url = "https://github.com/Exiv2/exiv2/issues/2670"
    filename = path("$tmp_path/exiv2-SonyDSC-HX60V.exv")
    commands = [
        '$exiv2 -u -PEkycv -g SonyMisc3c "$filename"',
        '$exiv2 -M"del Exif.SonyMisc3c.0x0054" '
        '-M"set Exif.SonyMisc3c.0x0054 73" "$filename"',
        '$exiv2 -u -PEkycv -g SonyMisc3c "$filename"',
    ]
    stdout = [""] * len(commands)
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Compare all group keys, types, counts, and raw values after reopening."""
        if i == 0:
            fields = [line.split() for line in got_stdout.splitlines()]
            self.assertIn(["Exif.SonyMisc3c.0x0054", "Byte", "1", "73"], fields)
            self.assertIn(["Exif.SonyMisc3c.0x0055", "Byte", "1", "255"], fields)
            self.original_group = got_stdout
        elif i == 2:
            self.assertEqual(self.original_group, got_stdout)
        else:
            self.assertEqual(expected_stdout, got_stdout)

# -*- coding: utf-8 -*-

from pathlib import Path
import shutil
import struct

from system_tests import CaseMeta, path


def sony_lens_dng():
    """Build a tiny DNG with independently encrypted A7RII lens coefficients."""
    values = (-32768, -249, -1, 0, 1, 248, 255, 32767)
    block = bytearray([0xa7] * 2048)
    for array, (offset, count) in enumerate(((0x64, 16), (0x368, 16), (0x39c, 32))):
        for i in range(count):
            struct.pack_into("<h", block, offset + 2 * i, values[(array + i) % 8])
    encrypted = bytearray(value ** 3 % 249 if value < 249 else value for value in block)
    encrypted[0] = 0x25
    origin = 0xffff0000
    directory = struct.pack("<HHHIII", 1, 0x9405, 7, len(encrypted), origin + 18, 0)
    record = b"II" + struct.pack(">I", origin) + directory + encrypted
    private = b"Adobe\0MakN" + struct.pack(">I", len(record)) + record
    entries = [
        (0x0100, 4, 1, struct.pack(">I", 1)),
        (0x0101, 4, 1, struct.pack(">I", 1)),
        (0x0102, 3, 1, struct.pack(">H", 8)),
        (0x0103, 3, 1, struct.pack(">H", 1)),
        (0x0106, 3, 1, struct.pack(">H", 1)),
        (0x010f, 2, 5, b"SONY\0"),
        (0x0110, 2, 10, b"ILCE-7RM2\0"),
        (0x0111, 4, 1, struct.pack(">I", 0)),
        (0x0115, 3, 1, struct.pack(">H", 1)),
        (0x0116, 4, 1, struct.pack(">I", 1)),
        (0x0117, 4, 1, struct.pack(">I", 1)),
        (0xc612, 1, 4, b"\x01\x04\0\0"),
        (0xc634, 1, len(private), private),
    ]

    # Keep all out-of-line values before the single pixel strip.
    start = 8 + 2 + 12 * len(entries) + 4
    table, payload = bytearray(), bytearray()
    for tag, kind, count, value in entries:
        table += struct.pack(">HHI", tag, kind, count)
        if tag == 0x0111:
            strip_position = 10 + len(table)
        if len(value) <= 4:
            table += value.ljust(4, b"\0")
        else:
            table += struct.pack(">I", start + len(payload))
            payload += value
            if len(payload) % 2:
                payload += b"\0"
    data = bytearray(b"MM\0*\0\0\0\x08" + struct.pack(">H", len(entries)) + table + b"\0" * 4 + payload)
    struct.pack_into(">I", data, strip_position, len(data))
    return data + b"\x7f"


class Sony9405bReadAndPreserve(metaclass=CaseMeta):
    """Expose lens arrays while retaining native and wrapped encrypted parents."""

    url = "https://github.com/Exiv2/exiv2/issues/2913"
    filenames = [path("$tmp_path/9405b_ILCA-68.exv"),
                 path("$tmp_path/9405b_ILCA-99M2.exv"),
                 path("$tmp_path/9405b_generated.dng")]
    commands, stderr, retval = [], [], []
    for index, filename in enumerate(filenames):
        parent = "Exif.Image.DNGPrivateData" if index == 2 else "Exif.Sony1.0x9405"
        commands += [
            f'$exiv2 -PEkycv -g Sony9405b "{filename}"',
            f'$exiv2 -u -PEkycv -K {parent} "{filename}"',
            f'$exiv2 -M"set Exif.Image.Artist {"a" * 600}" "{filename}"',
            f'$exiv2 -PEkycv -g Sony9405b "{filename}"',
            f'$exiv2 -u -PEkycv -K {parent} "{filename}"',
            f'$exiv2 -M"set Exif.Sony9405b.DistortionCorrParams SShort 1" "{filename}"',
            f'$exiv2 -PEkycv -g Sony9405b "{filename}"',
        ]
        reason = ("Setting Sony MakerNotes in Adobe DNG images is not supported" if index == 2 else
                  "Setting Sony9405b coefficients in Sony images is not supported")
        stderr += ["", "", "", "", "", f"Exiv2 exception in modify action for file {filename}:\n{reason}\n", ""]
        retval += [0, 0, 0, 0, 0, 1, 0]
    stdout = [""] * len(commands)

    def setUp(self):
        """Copy repository fixtures and generate a disposable wrapped sample."""
        self.paths = [Path(self.expand_variables(name)) for name in self.filenames]
        for target, source in zip(self.paths, ("exiv2-SonyILCA-68.exv", "exiv2-SonyILCA-99M2.exv")):
            shutil.copyfile(self.expand_variables(path(f"$data_path/{source}")), target)
        self.paths[2].write_bytes(sony_lens_dng())
        for filename in self.paths:
            self.addCleanup(filename.unlink, missing_ok=True)
        self.views, self.parents, self.before = {}, {}, {}

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Check array shape, exact parent retention, and unchanged failed output."""
        sample, operation = divmod(i, 7)
        if operation == 0:
            fields = [line.split() for line in got_stdout.splitlines()]
            self.assertEqual(3, len(fields))
            self.assertEqual(["16", "16", "32"], [field[2] for field in fields])
            self.assertTrue(all(field[1] == "SShort" for field in fields))
            self.views[sample] = got_stdout
        elif operation in (3, 6):
            self.assertEqual(self.views[sample], got_stdout)
            if operation == 6:
                self.assertEqual(self.before[sample], self.paths[sample].read_bytes())
        elif operation == 1:
            self.assertTrue(got_stdout.strip())
            self.parents[sample] = got_stdout
        elif operation == 4:
            self.assertEqual(self.parents[sample], got_stdout)
            self.before[sample] = self.paths[sample].read_bytes()
        else:
            self.assertEqual(expected_stdout, got_stdout)

# -*- coding: utf-8 -*-

from pathlib import Path
import struct

from system_tests import CaseMeta, path


def sony_dng_fixture():
    """Build a one-pixel big-endian DNG with relocated little-endian MakerNotes."""
    original_offset = 0xFFFF0000
    directory = struct.pack("<HHHIIHHIII", 2, 0x2011, 4, 1, 0,
                            0x9400, 7, 86, original_offset + 30, 0)
    array = bytearray(86)
    array[0], array[84], array[85] = 215, 73, 255
    struct.pack_into("<H", array, 71, 5304)
    encrypted = bytes(value ** 3 % 249 if value < 249 else value for value in array)

    # Adobe framing uses big endian even when the embedded MakerNote does not.
    maker_note = b"II" + struct.pack(">I", original_offset) + directory + encrypted
    private_data = (b"Adobe\0Test" + struct.pack(">I", 3) + b"odd\0" +
                    b"MakN" + struct.pack(">I", len(maker_note)) + maker_note +
                    b"SR2 " + struct.pack(">I", 4) + b"keep")
    entries = [
        (0x0100, 4, 1, struct.pack(">I", 1)),
        (0x0101, 4, 1, struct.pack(">I", 1)),
        (0x0102, 3, 1, struct.pack(">H", 8)),
        (0x0103, 3, 1, struct.pack(">H", 1)),
        (0x0106, 3, 1, struct.pack(">H", 1)),
        (0x010F, 2, 5, b"SONY\0"),
        (0x0110, 2, 10, b"ILCE-7RM2\0"),
        (0x0111, 4, 1, struct.pack(">I", 0)),
        (0x0115, 3, 1, struct.pack(">H", 1)),
        (0x0116, 4, 1, struct.pack(">I", 1)),
        (0x0117, 4, 1, struct.pack(">I", 1)),
        (0xC612, 1, 4, b"\x01\x04\0\0"),
        (0xC634, 1, len(private_data), private_data),
    ]

    # Place out-of-line values before the single image strip.
    start = 8 + 2 + 12 * len(entries) + 4
    table = bytearray()
    payload = bytearray()
    strip_position = None
    for tag, value_type, count, value in entries:
        table += struct.pack(">HHI", tag, value_type, count)
        if tag == 0x0111:
            strip_position = 10 + len(table)
        if len(value) <= 4:
            table += value.ljust(4, b"\0")
        else:
            table += struct.pack(">I", start + len(payload))
            payload += value
            if len(payload) % 2:
                payload += b"\0"
    data = bytearray(b"MM\0*\0\0\0\x08" + struct.pack(">H", len(entries)) +
                     table + b"\0\0\0\0" + payload)
    struct.pack_into(">I", data, strip_position, len(data))
    return data + b"\x7f"


class SonyDngMakerNoteRead(metaclass=CaseMeta):
    """Decode wrapped Sony fields and preserve the opaque parent during edits."""

    url = "https://github.com/Exiv2/exiv2/issues/2913"
    filename = path("$tmp_path/issue_2913.dng")
    commands = [
        '$exiv2 -u -PEkycv -g Sony "$filename"',
        '$exiv2 -M"set Exif.Image.Artist artist" "$filename"',
        '$exiv2 -u -PEkycv -g Sony "$filename"',
        '$exiv2 -PEkycv -K Exif.Image.Artist "$filename"',
        '$exiv2 -M"set Exif.Sony2.VignettingCorrection Long 1" "$filename"',
        '$exiv2 -u -PEkycv -g Sony "$filename"',
    ]
    stdout = [""] * len(commands)
    stderr = ["", "", "", "", """Exiv2 exception in modify action for file $filename:
Setting Sony MakerNotes in Adobe DNG images is not supported
""", ""]
    retval = [0, 0, 0, 0, 1, 0]

    def setUp(self):
        """Generate a disposable fixture without adding a camera image to Git."""
        self.image_path = Path(self.expand_variables(self.filename))
        self.image_path.write_bytes(sony_dng_fixture())
        self.addCleanup(self.image_path.unlink, missing_ok=True)

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Check exact field preservation and unchanged bytes after rejection."""
        fields = [line.split() for line in got_stdout.splitlines()]
        if i == 0:
            self.assertIn(["Exif.Sony2.VignettingCorrection", "Long", "1", "0"], fields)
            self.assertIn(["Exif.SonyMisc3c.SonyImageHeight", "Short", "1", "5304"], fields)
            self.original_group = got_stdout
        elif i in (2, 5):
            self.assertEqual(self.original_group, got_stdout)
            if i == 2:
                self.before_rejection = self.image_path.read_bytes()
            else:
                self.assertEqual(self.before_rejection, self.image_path.read_bytes())
        elif i == 3:
            self.assertEqual([["Exif.Image.Artist", "Ascii", "7", "artist"]], fields)
        else:
            self.assertEqual(expected_stdout, got_stdout)

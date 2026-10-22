# -*- coding: utf-8 -*-

from pathlib import Path
import struct

from system_tests import CaseMeta, path


def make_tiff(order):
    """Construct a root image and five distinct grayscale SubIFDs without Exiv2."""
    data = bytearray((b"II" if order == "<" else b"MM") + struct.pack(order + "HI", 42, 8))
    directories, targets, expected = [], [], []
    subifds = None
    for image in range(6):
        pixels = bytes([17 + image]) * (3 + image)
        expected.append(pixels)
        fields = [(0x00fe, 4, [int(image != 0)]), (0x0100, 4, [len(pixels)]),
                  (0x0101, 4, [1]), (0x0102, 3, [8]), (0x0103, 3, [1]),
                  (0x0106, 3, [1]), (0x0111, 4, [0]), (0x0115, 3, [1]),
                  (0x0116, 4, [1]), (0x0117, 4, [len(pixels)])]
        if image == 0:
            fields += [(0x013b, 2, b"seed\0"), (0x014a, 4, [0] * 5)]
        fields.sort()
        start = len(data)
        directories.append(start)
        data += struct.pack(order + "H", len(fields)) + bytes(12 * len(fields) + 4)
        for index, (tag, kind, values) in enumerate(fields):
            entry = start + 2 + 12 * index
            raw = values if kind == 2 else struct.pack(order + ("H" if kind == 3 else "I") * len(values), *values)
            struct.pack_into(order + "HHI", data, entry, tag, kind, len(values))
            value = entry + 8
            if len(raw) > 4:
                value = len(data)
                struct.pack_into(order + "I", data, entry + 8, value)
                data += raw + bytes(len(raw) % 2)
            else:
                data[value:value + 4] = raw.ljust(4, b"\0")
            if tag == 0x0111:
                targets.append(value)
            elif tag == 0x014a:
                subifds = value

    # Link the child array and place source pixels in their declared image order.
    struct.pack_into(order + "5I", data, subifds, *directories[1:])
    for target, pixels in zip(targets, expected):
        struct.pack_into(order + "I", data, target, len(data))
        data += pixels + bytes(len(pixels) % 2)
    return data, expected


def image_pixels(data):
    """Resolve bounded strip references independently of the library under test."""
    order = "<" if data[:2] == b"II" else ">"

    def directory(offset):
        """Read the small fixture's numeric TIFF entries and validate their ranges."""
        count, = struct.unpack_from(order + "H", data, offset)
        if count > 32 or offset + 2 + 12 * count + 4 > len(data):
            raise ValueError("invalid fixture directory")
        result = {}
        for index in range(count):
            entry = offset + 2 + 12 * index
            tag, kind, number = struct.unpack_from(order + "HHI", data, entry)
            width = {1: 1, 2: 1, 3: 2, 4: 4}[kind]
            length = width * number
            start = entry + 8 if length <= 4 else struct.unpack_from(order + "I", data, entry + 8)[0]
            if start + length > len(data):
                raise ValueError("invalid fixture value")
            if kind in (3, 4):
                result[tag] = struct.unpack_from(order + ("H" if kind == 3 else "I") * number, data, start)
        return result

    root = directory(struct.unpack_from(order + "I", data, 4)[0])
    images = [root] + [directory(offset) for offset in root[0x014a]]
    result = []
    for image in images:
        offset, = image[0x0111]
        length, = image[0x0117]
        if offset % 2 or offset + length > len(data):
            raise ValueError("invalid fixture image range")
        result.append(data[offset:offset + length])
    return result


class TiffSubIfdImageOrder(metaclass=CaseMeta):
    """Metadata edits must preserve each SubIFD's own pixels in both byte orders."""

    filenames = [path("$tmp_path/subifd_order_ii.tiff"), path("$tmp_path/subifd_order_mm.tiff")]
    commands = []
    for filename in filenames:
        commands += [f'$exiv2 -M"set Exif.Image.Artist {"a" * 600}" "{filename}"',
                     f'$exiv2 -M"set Exif.Image.Artist {"b" * 600}" "{filename}"',
                     f'$exiv2 -M"del Exif.Image.Artist" "{filename}"',
                     f'$exiv2 -M"set Exif.Image.ImageDescription repeated" "{filename}"']
    retval = [0] * len(commands)
    stdout = [""] * len(commands)
    stderr = [""] * len(commands)

    def setUp(self):
        """Create disposable fixtures and verify the initial independent references."""
        self.paths = [Path(self.expand_variables(name)) for name in self.filenames]
        self.expected = []
        for filename, order in zip(self.paths, ("<", ">")):
            data, expected = make_tiff(order)
            self.assertEqual(expected, image_pixels(data))
            filename.write_bytes(data)
            self.expected.append(expected)
            self.addCleanup(filename.unlink, missing_ok=True)

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Check referenced image bytes after each command, including silent writes."""
        self.assertEqual(expected_stdout, got_stdout)
        image = i // 4
        self.assertEqual(self.expected[image], image_pixels(self.paths[image].read_bytes()))

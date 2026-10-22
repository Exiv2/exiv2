# -*- coding: utf-8 -*-

import hashlib
from pathlib import Path
import struct
import tempfile

from system_tests import CaseMeta


def make_tiff(filename, order, count=0x10000000, kind=7):
    """Generate a large opaque value and one pixel without buffering the payload."""
    width = {1: 1, 3: 2, 7: 1}[kind]
    fields = [(256, 4, 1, 1), (257, 4, 1, 1), (258, 3, 1, 8),
              (259, 3, 1, 1), (262, 3, 1, 1), (273, 4, 1, 0),
              (277, 3, 1, 1), (278, 4, 1, 1), (279, 4, 1, 1),
              (315, 2, 2, b"A\0\0\0"), (37724, kind, count, 0)]
    pixel = 8 + 2 + 12 * len(fields) + 4
    payload = pixel + 2
    fields[5] = (273, 4, 1, pixel)
    fields[-1] = (37724, kind, count, payload)
    with filename.open("wb") as stream:
        stream.write((b"II" if order == "<" else b"MM") + struct.pack(order + "HIH", 42, 8, len(fields)))
        for tag, typ, number, value in fields:
            stream.write(struct.pack(order + "HHI", tag, typ, number))
            if isinstance(value, bytes):
                stream.write(value)
            elif typ == 3:
                stream.write(struct.pack(order + "H", value) + b"\0\0")
            else:
                stream.write(struct.pack(order + "I", value))
        stream.write(struct.pack(order + "I", 0) + b"\x7b\0")
        length = count * width
        stream.truncate(payload + length)
        markers = [(0, b"Adobe Photoshop Document Data Block\0"),
                   (length // 2, b"issue-3071-middle"),
                   (length - len(b"issue-3071-end!!!"), b"issue-3071-end!!!")]
        for offset, marker in markers:
            stream.seek(payload + offset)
            stream.write(marker)


def inspect_tiff(filename):
    """Resolve the generated fixture's IFD and hash the entire referenced value."""
    with filename.open("rb") as stream:
        header = stream.read(8)
        order = {b"II": "<", b"MM": ">"}[header[:2]]
        stream.seek(struct.unpack(order + "I", header[4:])[0])
        number, = struct.unpack(order + "H", stream.read(2))
        if number > 32:
            raise ValueError("unexpected fixture directory size")
        fields = {}
        for _ in range(number):
            entry = stream.read(12)
            tag, kind, count, offset = struct.unpack(order + "HHII", entry)
            length = count * {1: 1, 2: 1, 3: 2, 4: 4, 7: 1}[kind]
            fields[tag] = (kind, count, offset, entry[8:8 + min(length, 4)])
        if 37724 not in fields:
            raise AssertionError("metadata edit discarded Photoshop ImageSourceData")
        kind, count, offset, _ = fields[37724]
        length = count * {1: 1, 3: 2, 7: 1}[kind]
        if offset > filename.stat().st_size or length > filename.stat().st_size - offset:
            raise ValueError("payload is outside the file")
        stream.seek(offset)
        digest = hashlib.sha256()
        remaining = length
        while remaining:
            chunk = stream.read(min(remaining, 1024 * 1024))
            if not chunk:
                raise ValueError("truncated payload")
            digest.update(chunk)
            remaining -= len(chunk)
        stream.seek(fields[273][2])
        pixel = stream.read(fields[279][2])
        text = {}
        for tag in (270, 315):
            if tag in fields:
                _, count, offset, inline = fields[tag]
                if count > 4096:
                    raise ValueError("unexpected fixture text size")
                if count > 4:
                    stream.seek(offset)
                    inline = stream.read(count)
                text[tag] = inline.rstrip(b"\0").decode("ascii")
        return order, kind, fields[37724][1], digest.hexdigest(), pixel, text


class LargeTiffTags(metaclass=CaseMeta):
    """An ordinary metadata edit must retain a real 256 MiB tag in either byte order."""

    url = "https://github.com/Exiv2/exiv2/issues/3071"
    commands = []

    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="issue-3071-", dir=self.expand_variables("$tmp_path"))
        self.addCleanup(temporary.cleanup)
        self.filename = Path(temporary.name) / "large.tif"
        self.edits = ["set Exif.Image.Artist B", "set Exif.Image.Artist " + "x" * 600,
                      "del Exif.Image.Artist", "set Exif.Image.ImageDescription reopened"]
        binary = self.expand_variables("$exiv2")
        self.commands = [f'"{binary}" -M"{edit}" "{self.filename}"' for edit in self.edits] * 2
        self.stdout = self.stderr = [""] * len(self.commands)
        self.retval = [0] * len(self.commands)
        self.stdin = [None] * len(self.commands)
        self.make_fixture("<")

    def make_fixture(self, order):
        make_tiff(self.filename, order)
        self.order, _, _, self.digest, _, _ = inspect_tiff(self.filename)

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        order, kind, count, digest, pixel, text = inspect_tiff(self.filename)
        self.assertEqual((self.order, 7, 0x10000000, self.digest, b"\x7b"),
                         (order, kind, count, digest, pixel))
        artist = ["B", "x" * 600, None, None][i % 4]
        self.assertEqual(artist, text.get(315))
        self.assertEqual("reopened" if i % 4 == 3 else None, text.get(270))
        self.assertEqual(expected_stderr, got_stderr)

    def post_command_hook(self, i, command):
        if i == 3:
            # Reuse the path so only one large fixture exists at a time.
            self.filename.unlink()
            self.make_fixture(">")

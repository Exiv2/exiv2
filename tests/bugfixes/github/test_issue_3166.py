# -*- coding: utf-8 -*-

from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

import system_tests


@unittest.skipUnless(
    system_tests.BT.verbose_version().get("enable_video") == "1",
    "Nikon video regressions require video support",
)
class NikonVideoMetadata(unittest.TestCase):
    """Read synthetic Nikon MP4 metadata without footage or an XMP toolkit."""

    def setUp(self):
        """Own disposable fixtures independently of repository samples."""
        temporary = tempfile.TemporaryDirectory(prefix="exiv2-3166-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)

    @staticmethod
    def atom(kind, data):
        """Encode an ordinary bounded atom for the metadata-only movie."""
        return struct.pack(">I4s", len(data) + 8, kind.encode("ascii")) + data

    @staticmethod
    def record(tag, field_type, count, data):
        """Frame a Nikon record without assuming its decoded value."""
        return struct.pack(">IHH", tag, field_type, count) + data

    def movie(self, records):
        """Wrap records in the recognized Nikon metadata hierarchy."""
        return self.atom("ftyp", b"mp42\0\0\0\1mp42") + self.atom("free", bytes(128)) + self.atom(
            "moov", self.atom("udta", self.atom("NCDT", self.atom("NCTG", records)))
        )

    def run_movie(self, data, expected=0):
        """Check all-metadata CLI output and its explicit failure status."""
        path = self.directory / "synthetic.mp4"
        path.write_bytes(data)
        result = subprocess.run(
            [system_tests.exiv2, "-pa", str(path)], capture_output=True,
            text=True, check=False, timeout=30,
        )
        self.assertEqual(expected, result.returncode, result.stderr)
        if expected == 0:
            self.assertEqual("", result.stderr)
        return result.stdout

    def test_signed_short_and_following_record(self):
        """A signed scalar must not consume the following record."""
        for value in (-32768, -1, 0, 32767):
            with self.subTest(value=value):
                records = self.record(0x0200000B, 8, 1, struct.pack(">h", value))
                records += self.record(2, 2, 9, b"sentinel\0")
                output = self.run_movie(self.movie(records))
                fields = {line.split()[0]: line.split(maxsplit=3)[3] for line in output.splitlines()}
                self.assertEqual(str(value), fields["Xmp.video.WhiteBalanceFineTune"])
                self.assertEqual("sentinel", fields["Xmp.video.Model"])

    def test_modern_picture_control(self):
        """Modern text uses versioned offsets, retaining native casing."""
        for version in (b"0302", b"0310"):
            with self.subTest(version=version):
                data = bytearray(108)
                data[:4] = version
                data[4:8] = b"PAD!"
                data[8:13] = b"N-LOG"
                data[28:33] = b"N-LOG"
                output = self.run_movie(self.movie(self.record(0x02000023, 7, len(data), data)))
                fields = {line.split()[0]: line.split(maxsplit=3)[3] for line in output.splitlines()}
                self.assertEqual(version.decode(), fields["Xmp.video.PictureControlVersion"])
                self.assertEqual("N-LOG", fields["Xmp.video.PictureControlName"])
                self.assertEqual("N-LOG", fields["Xmp.video.PictureControlBase"])
                self.assertNotIn("Xmp.video.Sharpness", fields)

    def test_malformed_record_rejects(self):
        """A claimed field must fit its own Nikon payload."""
        malformed = self.record(2, 2, 100, b"x")
        output = self.run_movie(self.movie(malformed), expected=1)
        self.assertEqual("", output)

    def test_timecode_url_still_reads(self):
        """Preserve #3426's skipping of a timecode track's URL payload."""
        handler = self.atom("hdlr", bytes(8) + b"tmcd" + bytes(12) + b"TimeCodeHandler\0")
        reference = self.atom("dref", bytes(4) + struct.pack(">I", 1) + self.atom("url ", struct.pack(">I", 1)))
        track = self.atom("trak", self.atom("mdia", handler + self.atom("minf", self.atom("dinf", reference))))
        self.run_movie(self.atom("ftyp", b"mp42\0\0\0\1mp42") + self.atom("moov", track))

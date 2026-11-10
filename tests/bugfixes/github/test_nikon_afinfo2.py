# SPDX-License-Identifier: GPL-2.0-or-later

"""AFInfo2 geometry using the layouts documented in ExifTool's Nikon.pm."""

from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory

from system_tests import CaseMeta


GEOMETRY = dict(zip(
    ("AFImageWidth", "AFImageHeight", "AFAreaXPosition", "AFAreaYPosition",
     "AFAreaWidth", "AFAreaHeight"),
    (6048, 4024, 4590, 2580, 285, 308),
))


def af_payload(version, order):
    base, size = {"01": (16, 30), "03": (42, 64), "04": (62, 84)}[version[:2]]
    if version == "0101":
        base, size = 70, 84
    data = bytearray((i * 13 + 7) % 256 for i in range(size))
    data[:4] = version.encode("ascii")
    data[4:8] = bytes((1, 1, 0, 0) if version.startswith("01") else (2, 193, 8, 1))
    for i, value in enumerate(GEOMETRY.values()):
        struct.pack_into(order + "H", data, base + i * 2, value)
    if version.startswith("03"):
        data[56] = 3
    if version.startswith("04"):
        data[74] = 1
    return bytes(data)


def exv_file(payload, order, outer_order="<"):
    def header(endian):
        return (b"II" if endian == "<" else b"MM") + struct.pack(endian + "HI", 42, 8)

    def entry(endian, tag, kind, count, offset):
        return struct.pack(endian + "HHII", tag, kind, count, offset)

    # MakerNote offsets are relative to its embedded TIFF header, not IFD0.
    maker = (b"Nikon\0\x02\0\0\0" + header(order) + struct.pack(order + "H", 1)
             + entry(order, 0x00b7, 7, len(payload), 26)
             + bytes(4) + payload)
    tiff = (header(outer_order) + struct.pack(outer_order + "H", 2)
            + entry(outer_order, 0x010f, 2, 6, 38)
            + entry(outer_order, 0x8769, 4, 1, 44) + bytes(4) + b"NIKON\0"
            + struct.pack(outer_order + "H", 1)
            + entry(outer_order, 0x927c, 7, len(maker), 62) + bytes(4) + maker)
    app1 = b"Exif\0\0" + tiff
    return b"\xff\x01Exiv2\xff\xe1" + struct.pack(">H", len(app1) + 2) + app1 + b"\xff\xd9"


class NikonAfInfo2(metaclass=CaseMeta):
    commands = ["$exiv2 -Pv -K Exif.NikonAf23.AFAreaXPosition "
                "-K Exif.NikonAf23.AFAreaYPosition $data_path/NikonZ6.exv"]
    stdout = ["4590\n2580\n"]

    def setUp(self):
        self.temp = TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.filename = Path(self.temp.name) / "nikon.exv"
        self.exiv2 = self.expand_variables("$exiv2")

    def run_exiv2(self, *args):
        proc = subprocess.run([self.exiv2, *args, str(self.filename)],
                              capture_output=True, text=True, timeout=30)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertEqual(proc.stderr, "")
        return proc.stdout

    def read_tags(self):
        return {parts[0]: parts[1] if len(parts) > 1 else ""
                for line in self.run_exiv2("-Pkv").splitlines()
                if (parts := line.split(maxsplit=1))}

    def test_geometry_versions_and_byte_orders(self):
        for version, group in (("0100", "NikonAf2"), ("0101", "NikonAf22"),
                               ("0300", "NikonAf23"), ("0301", "NikonAf23"),
                               ("0400", "NikonAf24"), ("0401", "NikonAf24"),
                               ("0402", "NikonAf24")):
            for order in ("<", ">"):
                for outer_order in ("<", ">"):
                    with self.subTest(version=version, order=order, outer_order=outer_order):
                        self.filename.write_bytes(exv_file(af_payload(version, order), order, outer_order))
                        tags = self.read_tags()
                        for tag, value in GEOMETRY.items():
                            self.assertEqual(tags[f"Exif.{group}.{tag}"], str(value))
                        if version.startswith(("03", "04")):
                            self.assertEqual(tags[f"Exif.{group}.AFDetectionMethod"], "2")
                            self.assertEqual(tags[f"Exif.{group}.AFCoordinatesAvailable"], "1")
                            self.assertIn("Hybrid", self.run_exiv2("-pa", "--grep", "AFDetectionMethod"))
                            self.assertIn("Single", self.run_exiv2("-pa", "--grep", "AFAreaMode"))
                        if version.startswith("03"):
                            self.assertEqual(tags[f"Exif.{group}.FocusPointSchema"], "8")
                            self.assertEqual(tags[f"Exif.{group}.PrimaryAFPoint"], "3")
                        if version.startswith("04"):
                            self.assertIn("Focus", self.run_exiv2("-pa", "--grep", "FocusResult"))

    def test_camera_z6(self):
        source = self.expand_variables("$data_path/NikonZ6.exv")
        self.filename.write_bytes(Path(source).read_bytes())
        tags = self.read_tags()
        for tag, value in GEOMETRY.items():
            self.assertEqual(tags[f"Exif.NikonAf23.{tag}"], str(value))

    def test_coordinate_availability(self):
        for version, group in (("0300", "NikonAf23"), ("0400", "NikonAf24")):
            data = bytearray(af_payload(version, "<"))
            data[7] = 0
            self.filename.write_bytes(exv_file(data, "<"))
            self.assertEqual(self.read_tags()[f"Exif.{group}.AFCoordinatesAvailable"], "0")
            self.assertIn("No", self.run_exiv2("-pa", "--grep", "AFCoordinatesAvailable"))

    def test_unknown_versions_remain_opaque(self):
        for version in (b"0200", b"0302", b"0403", b"0500", b"9999"):
            self.filename.write_bytes(exv_file(version + bytes(80), "<"))
            tags = self.read_tags()
            self.assertIn("Exif.Nikon3.AFInfo2", tags)
            self.assertFalse(any("AFAreaXPosition" in key for key in tags))

    def test_short_blocks_do_not_read_coordinates_past_end(self):
        for version, limit in (("0300", 46), ("0400", 66)):
            for size in (4, 8, limit):
                self.filename.write_bytes(exv_file(af_payload(version, "<")[:size], "<"))
                tags = self.read_tags()
                self.assertFalse(any("AFAreaXPosition" in key for key in tags))
                self.assertFalse(any("AFAreaYPosition" in key for key in tags))

    def test_write_preserves_unknown_bytes(self):
        for version, group, offset in (("0300", "NikonAf23", 46), ("0402", "NikonAf24", 66)):
            for order in ("<", ">"):
                with self.subTest(version=version, order=order):
                    payload = af_payload(version, order)
                    self.filename.write_bytes(exv_file(payload, order))
                    # Force an intrusive rewrite by adding a large new value.
                    self.run_exiv2("-Mset Exif.Image.Artist " + "a" * 512)
                    self.assertIn(payload, self.filename.read_bytes())
                    self.run_exiv2(f"-Mset Exif.{group}.AFAreaXPosition 1234")
                    expected = bytearray(payload)
                    struct.pack_into(order + "H", expected, offset, 1234)
                    self.assertIn(bytes(expected), self.filename.read_bytes())
                    self.assertEqual(self.read_tags()[f"Exif.{group}.AFAreaXPosition"], "1234")

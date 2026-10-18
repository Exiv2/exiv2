# -*- coding: utf-8 -*-

import hashlib
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

import system_tests


# Frame generated container fields independently of the library's BMFF reader.
def box(kind, payload):
    return struct.pack(">I4s", len(payload) + 8, kind) + payload


# Embed an existing small, valid JPEG in a single-sample Canon video track.
def cr3(jpeg, wide_offsets=True, variable_size=False):
    ftyp = box(b"ftyp", b"crx \0\0\0\0crx ")
    mdat = box(b"mdat", jpeg)
    prefix = bytearray(82)
    struct.pack_into(">H", prefix, 6, 1)  # Local data-reference index.
    struct.pack_into(">HH", prefix, 24, 150, 91)
    description = box(b"CRAW", prefix + box(b"JPEG", bytes(4)))

    # Resolve the JPEG through sample tables, rather than its position in mdat.
    stsd = box(b"stsd", struct.pack(">II", 0, 1) + description)
    stsc = box(b"stsc", struct.pack(">IIIII", 0, 1, 1, 1, 1))
    sizes = struct.pack(">III", 0, 0 if variable_size else len(jpeg), 1)
    if variable_size:
        sizes += struct.pack(">I", len(jpeg))
    stsz = box(b"stsz", sizes)
    offset = struct.pack(">Q" if wide_offsets else ">I", len(ftyp) + 8)
    chunks = box(b"co64" if wide_offsets else b"stco", struct.pack(">II", 0, 1) + offset)
    stbl = box(b"stbl", stsd + stsc + stsz + chunks)
    dref = box(b"dref", struct.pack(">II", 0, 1) + box(b"url ", struct.pack(">I", 1)))
    minf = box(b"minf", box(b"dinf", dref) + stbl)
    hdlr = box(b"hdlr", bytes(8) + b"vide" + bytes(12))
    return ftyp + mdat + box(b"moov", box(b"trak", box(b"mdia", hdlr + minf)))


# Exercise public listing/extraction without adding large camera files to the suite.
@unittest.skipUnless(system_tests.BT.verbose_version().get("enable_bmff") == "1",
                     "requires BMFF support")
class Cr3FullPreview(unittest.TestCase):
    # Own disposable containers and preserve the original JPEG fixture.
    def setUp(self):
        directory = tempfile.TemporaryDirectory(prefix="exiv2-cr3-", dir=system_tests.tmp_path)
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.exiv2 = str(Path(system_tests.BT.Config.bin_dir) / "exiv2")
        self.jpeg = (Path(system_tests.data_path) / "exiv2-empty.jpg").read_bytes()

    # Run the intended build with bounded execution and captured diagnostics.
    def run_exiv2(self, *args):
        return subprocess.run([self.exiv2, *map(str, args)], capture_output=True, timeout=30)

    # Both standard size/offset encodings must expose and extract the exact JPEG.
    def test_list_and_extract_full_jpeg(self):
        for wide, variable in [(True, False), (False, True)]:
            with self.subTest(wide_offsets=wide, variable_size=variable):
                path = self.root / f"generated-{wide}-{variable}.CR3"
                path.write_bytes(cr3(self.jpeg, wide, variable))
                before = hashlib.sha256(path.read_bytes()).digest()

                listing = self.run_exiv2("-pp", path)
                self.assertEqual(listing.returncode, 0, listing.stderr)
                self.assertEqual(listing.stdout, (
                    f"Preview 1: image/jpeg, 150x91 pixels, {len(self.jpeg)} bytes\n"
                ).encode())
                extracted = self.run_exiv2("-ep", path)
                self.assertEqual(extracted.returncode, 0, extracted.stderr)
                self.assertEqual(path.with_name(path.stem + "-preview1.jpg").read_bytes(), self.jpeg)
                self.assertEqual(hashlib.sha256(path.read_bytes()).digest(), before)

    # Additional preview reading must not enable CR3 metadata writing.
    def test_cr3_remains_read_only(self):
        path = self.root / "read-only.CR3"
        data = cr3(self.jpeg)
        path.write_bytes(data)

        result = self.run_exiv2("-M", "set Exif.Image.Artist unchanged", path)

        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(path.read_bytes(), data)

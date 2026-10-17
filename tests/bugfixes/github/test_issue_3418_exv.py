# SPDX-License-Identifier: GPL-2.0-or-later

import os
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

import system_tests


class MultiSegmentExif(unittest.TestCase):
    """Exercise synthetic JPEG/EXV continuations through the command-line interface."""

    def test_reads_and_rejects_writes(self):
        """Read a value beyond the first APP1 and preserve source bytes on write rejection."""
        executable = os.path.join(system_tests.BT.Config.bin_dir, "exiv2")
        # An IFD near the TIFF tail models GPS fields located beyond a large MakerNote.
        tiff = bytearray(70000)
        struct.pack_into("<2sHI", tiff, 0, b"II", 42, 69000)
        # Write the IFD entry explicitly: tag, type, count, inline value, next IFD.
        tiff[69000:69018] = struct.pack("<HHHI4sI", 1, 0x010F, 2, 4, b"GPS\0", 0)

        with TemporaryDirectory() as directory:
            for suffix, header in (("exv", b"\xff\x01Exiv2"), ("jpg", b"\xff\xd8")):
                with self.subTest(container=suffix):
                    content = bytearray(header)
                    for offset in range(0, len(tiff), 65527):
                        payload = b"Exif\0\0" + tiff[offset : offset + 65527]
                        content += b"\xff\xe1" + struct.pack(">H", len(payload) + 2) + payload
                    content += b"\xff\xd9"
                    path = Path(directory) / ("fragmented." + suffix)
                    path.write_bytes(content)

                    result = subprocess.run(
                        [executable, "-K", "Exif.Image.Make", "-Pv", str(path)],
                        capture_output=True, text=True, check=False,
                    )
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout.strip(), "GPS")
                    self.assertEqual(result.stderr, "")

                    for arguments in (["-Mset Exif.Image.Artist edited"], ["-da"]):
                        result = subprocess.run(
                            [executable, *arguments, str(path)],
                            capture_output=True, text=True, check=False,
                        )
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("multi-segment Exif", result.stderr)
                        self.assertEqual(path.read_bytes(), content)

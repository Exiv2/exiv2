# -*- coding: utf-8 -*-
import hashlib
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET

import system_tests


@unittest.skipUnless(system_tests.BT.verbose_version().get("have_xmptoolkit") == "1", "Requires XMP toolkit")
class SafeSidecarExport(unittest.TestCase):
    """Malformed native strings must not publish partial XMP sidecars."""

    def test_invalid_text_and_valid_control(self):
        samples = [
            (0x9286, 7, bytes.fromhex("0a0000004a4b4a4b2703ab"), True),
            (0x0110, 2, bytes.fromhex("e580f000"), True),
            (0x8298, 2, b"\xa9 Example\0", True),
            (0x8298, 2, "© Example\0".encode(), False),
        ]
        original = (Path(system_tests.BT.Config.data_dir) / "exiv2-empty.jpg").read_bytes()
        seed = (
            '<?xml version="1.0"?><x:xmpmeta xmlns:x="adobe:ns:meta/">'
            '<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">'
            '<rdf:Description rdf:about="" xmlns:xmp="http://ns.adobe.com/xap/1.0/" '
            'xmp:Label="original-canary"/></rdf:RDF></x:xmpmeta>'
        ).encode()
        with tempfile.TemporaryDirectory(prefix="exiv2-9551-") as directory:
            image = Path(directory) / "sample.jpg"
            sidecar = image.with_suffix(".xmp")
            for tag, type_id, raw, invalid in samples:
                tiff = b"II" + struct.pack("<HI", 42, 8)
                if tag == 0x9286:
                    tiff += struct.pack("<HHHIII", 1, 0x8769, 4, 1, 26, 0)
                offset = len(tiff) + 18
                value = raw.ljust(4, b"\0") if len(raw) <= 4 else struct.pack("<I", offset)
                tiff += struct.pack("<HHHI", 1, tag, type_id, len(raw)) + value + struct.pack("<I", 0)
                if len(raw) > 4:
                    tiff += raw
                payload = b"Exif\0\0" + tiff
                jpeg = original[:2] + b"\xff\xe1" + struct.pack(">H", len(payload) + 2) + payload + original[2:]
                image.write_bytes(jpeg)
                with self.subTest(tag=tag, invalid=invalid, stdout=True):
                    result = subprocess.run(
                        [system_tests.exiv2, "-eX-", str(image)],
                        capture_output=True, timeout=30,
                    )
                    self.assertEqual(1 if invalid else 0, result.returncode, result.stderr)
                    if invalid:
                        self.assertEqual(b"", result.stdout)
                    else:
                        self.assertIn("© Example", list(ET.fromstring(result.stdout).itertext()))
                for existing in (False, True):
                    with self.subTest(tag=tag, invalid=invalid, existing=existing):
                        sidecar.unlink(missing_ok=True)
                        if existing:
                            sidecar.write_bytes(seed)
                        result = subprocess.run(
                            [system_tests.exiv2, "-f", "-eX", str(image)],
                            capture_output=True, timeout=30,
                        )
                        self.assertEqual(1 if invalid else 0, result.returncode, result.stderr)
                        self.assertEqual(hashlib.sha256(jpeg).digest(), hashlib.sha256(image.read_bytes()).digest())
                        if invalid:
                            self.assertIn(b"Failed to encode XMP metadata", result.stderr)
                            self.assertEqual(existing, sidecar.exists())
                            if existing:
                                self.assertEqual(seed, sidecar.read_bytes())
                        else:
                            root = ET.fromstring(sidecar.read_bytes())
                            self.assertIn("© Example", list(root.itertext()))

    def test_empty_and_raw_exports(self):
        original = (Path(system_tests.BT.Config.data_dir) / "exiv2-empty.jpg").read_bytes()
        packet = (
            '<?xml version="1.0"?><x:xmpmeta xmlns:x="adobe:ns:meta/">'
            '<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">'
            '<rdf:Description rdf:about="" xmlns:xmp="http://ns.adobe.com/xap/1.0/" '
            'xmp:Label="raw-canary"/></rdf:RDF></x:xmpmeta>'
        ).encode()
        with tempfile.TemporaryDirectory(prefix="exiv2-9551-") as directory:
            image = Path(directory) / "sample.jpg"
            sidecar = image.with_suffix(".xmp")
            image.write_bytes(original)
            result = subprocess.run([system_tests.exiv2, "-eX", str(image)], capture_output=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(54, sidecar.stat().st_size)

            payload = b"http://ns.adobe.com/xap/1.0/\0" + packet
            image.write_bytes(original[:2] + b"\xff\xe1" + struct.pack(">H", len(payload) + 2) + payload + original[2:])
            for existing in (False, True):
                with self.subTest(existing=existing):
                    sidecar.unlink(missing_ok=True)
                    if existing:
                        sidecar.write_bytes(packet.replace(b"raw-canary", b"original-canary"))
                    # Select both sidecar output and the raw-packet shortcut.
                    result = subprocess.run(
                        [system_tests.exiv2, "-f", "-eX", "-eXX", str(image)], capture_output=True, timeout=30,
                    )
                    self.assertEqual(0, result.returncode, result.stderr)
                    self.assertEqual(packet, sidecar.read_bytes())

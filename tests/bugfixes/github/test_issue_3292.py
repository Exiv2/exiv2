# -*- coding: utf-8 -*-

import hashlib
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET

import system_tests


EXIF_NS = "http://ns.adobe.com/exif/1.0/"
RDF_NS = "http://www.w3.org/1999/02/22-rdf-syntax-ns#"


@unittest.skipUnless(
    system_tests.BT.verbose_version().get("have_xmptoolkit") == "1",
    "DeviceSettings sidecar regressions require the XMP toolkit",
)
class DeviceSettingsSidecar(unittest.TestCase):
    """Exercise #3292 using synthetic metadata and an existing empty JPEG."""

    def setUp(self):
        """Own a disposable directory without modifying the original fixture."""
        temporary = tempfile.TemporaryDirectory(prefix="exiv2-3292-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.exiv2 = system_tests.exiv2

    def run_exiv2(self, *arguments, expected=0):
        """Run the selected build and require explicit command success or failure."""
        result = subprocess.run(
            [self.exiv2, *(str(arg) for arg in arguments)],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
            encoding="utf-8", timeout=30,
        )
        self.assertEqual(expected, result.returncode, result.stderr)
        if expected == 0:
            self.assertEqual("", result.stderr)
        return result

    def fixture(self, order, existing_xmp=True, zero=False):
        """Embed a real Exif sub-IFD and an optional structured XMP APP1."""
        columns, rows = (0, 0) if zero else (2, 1)
        settings = [] if zero else ["Auto", "Žluť"]
        native = struct.pack(order + "HH", columns, rows)
        for value in settings:
            native += b"\xfe\xff" + value.encode("utf-16-be") + b"\0\0"

        # Offsets are relative to the TIFF header, including the Exif subdirectory.
        tiff = (b"II" if order == "<" else b"MM") + struct.pack(order + "HI", 42, 8)
        tiff += struct.pack(order + "HHHIII", 1, 0x8769, 4, 1, 26, 0)
        entry_value = native if len(native) <= 4 else struct.pack(order + "I", 44)
        tiff += struct.pack(order + "HHHI", 1, 0xA40B, 7, len(native))
        tiff += entry_value + struct.pack(order + "I", 0)
        if len(native) > 4:
            tiff += native

        def segment(payload):
            """Frame a small metadata payload as one JPEG APP1 segment."""
            return b"\xff\xe1" + struct.pack(">H", len(payload) + 2) + payload

        metadata = segment(b"Exif\0\0" + tiff)
        if existing_xmp:
            values = "" if zero else (
                "<exif:Values><rdf:Seq><rdf:li>Auto</rdf:li>"
                "<rdf:li>Žluť</rdf:li></rdf:Seq></exif:Values>"
            )
            packet = (
                '<x:xmpmeta xmlns:x="adobe:ns:meta/">'
                f'<rdf:RDF xmlns:rdf="{RDF_NS}"><rdf:Description rdf:about="" '
                f'xmlns:exif="{EXIF_NS}">'
                '<exif:DeviceSettingDescription rdf:parseType="Resource">'
                f'<exif:Columns>{columns}</exif:Columns><exif:Rows>{rows}</exif:Rows>'
                f'{values}</exif:DeviceSettingDescription>'
                '</rdf:Description></rdf:RDF></x:xmpmeta>'
            )
            metadata += segment(b"http://ns.adobe.com/xap/1.0/\0" + packet.encode("utf-8"))
        original = (Path(system_tests.BT.Config.data_dir) / "exiv2-empty.jpg").read_bytes()
        image = self.directory / "sample.jpg"
        image.write_bytes(original[:2] + metadata + original[2:])
        return image

    def device_settings(self, path):
        """Read semantic fields from either compact-attribute or expanded XMP."""
        root = ET.fromstring(path.read_bytes())
        node = root.find(f".//{{{EXIF_NS}}}DeviceSettingDescription")
        self.assertIsNotNone(node)
        description = node.find(f"{{{RDF_NS}}}Description")
        owner = node if description is None else description

        def dimension(name):
            """Accept both legal serializations of an integer struct field."""
            tag = f"{{{EXIF_NS}}}{name}"
            child = owner.find(tag)
            return owner.get(tag) if child is None else child.text

        values = [value.text or "" for value in node.findall(f".//{{{RDF_NS}}}li")]
        return dimension("Columns"), dimension("Rows"), values

    def test_export_reinsert_and_reexport_preserve_values(self):
        """Preserve both byte orders, native-only input, and the reporter's zero case."""
        for order in ("<", ">"):
            for existing_xmp in (False, True):
                for zero in (False, True):
                    with self.subTest(order=order, xmp=existing_xmp, zero=zero):
                        image = self.fixture(order, existing_xmp, zero)
                        sidecar = image.with_suffix(".xmp")
                        sidecar.unlink(missing_ok=True)
                        source = hashlib.sha256(image.read_bytes()).digest()
                        native = self.run_exiv2("-pv", "-K", "Exif.Photo.DeviceSettingDescription", image).stdout
                        self.run_exiv2("-eX", image)
                        self.assertEqual(source, hashlib.sha256(image.read_bytes()).digest())
                        expected = ("0", "0", []) if zero else ("2", "1", ["Auto", "Žluť"])
                        self.assertEqual(expected, self.device_settings(sidecar))

                        # Reading the sidecar must reconstruct native Exif even without the source JPEG.
                        reconstructed = self.run_exiv2("-pv", "-K", "Exif.Photo.DeviceSettingDescription", sidecar)
                        self.assertIn("Undefined", reconstructed.stdout)
                        self.run_exiv2("-iX", image)
                        restored = self.run_exiv2("-pv", "-K", "Exif.Photo.DeviceSettingDescription", image).stdout
                        # Native dimensions retain TIFF order; string signatures may normalize on import.
                        self.assertIn("Undefined", restored)
                        original_bytes = native.split("Undefined", 1)[1].split()[1:]
                        restored_bytes = restored.split("Undefined", 1)[1].split()[1:]
                        self.assertEqual(original_bytes[:4], restored_bytes[:4])
                        self.run_exiv2("-f", "-eX", image)
                        self.assertEqual(expected, self.device_settings(sidecar))

    def test_sidecar_edits_update_the_intended_representation(self):
        """Reconstructed native Exif must not undo subsequent structured XMP edits."""
        image = self.fixture(">")
        sidecar = image.with_suffix(".xmp")
        self.run_exiv2("-eX", image)
        self.run_exiv2("-Mset Xmp.exif.DeviceSettingDescription/exif:Columns 7", sidecar)
        self.assertEqual(("7", "1", ["Auto", "Žluť"]), self.device_settings(sidecar))

        # An explicit native edit should instead refresh the XMP projection.
        native = struct.pack("<HH", 9, 3)
        self.run_exiv2("-Mset Exif.Photo.DeviceSettingDescription " + " ".join(str(b) for b in native), sidecar)
        self.assertEqual(("9", "3", []), self.device_settings(sidecar))

        # Removing the subtree must not recreate it from an unchanged derived Exif tag.
        self.run_exiv2("-Mset Xmp.xmp.Label retained-canary", sidecar)
        self.run_exiv2("-Mdel Xmp.exif.DeviceSettingDescription", sidecar)
        root = ET.fromstring(sidecar.read_bytes())
        self.assertIsNone(root.find(f".//{{{EXIF_NS}}}DeviceSettingDescription"))

    def test_encoding_failure_reports_nonzero_without_changing_existing_bytes(self):
        """An independently invalid XMP subtree must reach the CLI as failure."""
        for existing in (False, True):
            with self.subTest(existing=existing):
                image = self.directory / "invalid.jpg"
                image.write_bytes((Path(system_tests.BT.Config.data_dir) / "exiv2-empty.jpg").read_bytes())
                source = image.read_bytes()
                sidecar = image.with_suffix(".xmp")
                sidecar.unlink(missing_ok=True)
                if existing:
                    sidecar.write_text(
                        '<?xml version="1.0"?><x:xmpmeta xmlns:x="adobe:ns:meta/">'
                        f'<rdf:RDF xmlns:rdf="{RDF_NS}"><rdf:Description rdf:about=""/>'
                        '</rdf:RDF></x:xmpmeta>', encoding="utf-8",
                    )
                before = sidecar.read_bytes() if existing else None
                result = self.run_exiv2(
                    "-f", "-Madd Xmp.xmpMM.DerivedFrom/stRef:documentID child-canary",
                    "-Madd Xmp.xmpMM.DerivedFrom XmpText scalar-conflict", "-eX", image,
                    expected=1,
                )
                self.assertIn("Failed to encode XMP metadata", result.stderr)
                self.assertEqual(source, image.read_bytes())
                if existing:
                    self.assertEqual(before, sidecar.read_bytes())

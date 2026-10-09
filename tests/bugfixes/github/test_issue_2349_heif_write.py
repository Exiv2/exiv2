# -*- coding: utf-8 -*-

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

import system_tests


@unittest.skipUnless(system_tests.BT.verbose_version().get("enable_bmff") == "1",
                     "requires BMFF support")
class HeifMetadataWriting(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="exiv2-heif-", dir=system_tests.tmp_path)
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.exiv2 = str(Path(system_tests.BT.Config.bin_dir) / "exiv2")

    def copy(self, filename):
        target = self.root / filename
        shutil.copyfile(Path(system_tests.data_path) / filename, target)
        return target

    def run_exiv2(self, *args, success=True):
        result = subprocess.run([self.exiv2, *map(str, args)], capture_output=True, timeout=30)
        if success:
            self.assertEqual(result.returncode, 0, result.stderr.decode(errors="replace"))
        else:
            self.assertNotEqual(result.returncode, 0)
        return result.stdout

    @unittest.skipUnless(system_tests.BT.verbose_version().get("have_xmptoolkit") == "1",
                         "requires structured XMP editing")
    def test_extract_clear_and_insert_metadata(self):
        target = self.copy("Stonehenge.heic")
        self.run_exiv2("-M", "set Exif.Image.Artist HEIF_EXIF_CANARY_627c",
                       "-M", "set Xmp.dc.source HEIF_XMP_CANARY_691d", target)
        self.run_exiv2("-ea", target)
        self.assertTrue(target.with_suffix(".exv").exists())
        self.run_exiv2("-de", target)
        self.run_exiv2("-dx", target)
        data = target.read_bytes()
        self.assertNotIn(b"HEIF_EXIF_CANARY_627c", data)
        self.assertNotIn(b"HEIF_XMP_CANARY_691d", data)
        self.run_exiv2("-ia", target)
        self.assertEqual(self.run_exiv2("-K", "Exif.Image.Artist", "-Pv", target),
                         b"HEIF_EXIF_CANARY_627c\n")
        self.assertEqual(self.run_exiv2("-K", "Xmp.dc.source", "-Pv", target),
                         b"HEIF_XMP_CANARY_691d\n")
        self.assertFalse(list(self.root.glob(".exiv2-heif-*")))

    def test_makernote_values_shrink_and_disappear(self):
        target = self.copy("Canon.HIF")
        canary = "PRIVATE_CANON_OWNER_44bea7"
        self.run_exiv2("-M", "set Exif.Canon.OwnerName " + "x" * 1000 + canary, target)
        self.assertIn(canary.encode(), target.read_bytes())
        self.run_exiv2("-M", "set Exif.Canon.OwnerName owner", target)
        self.assertNotIn(canary.encode(), target.read_bytes())
        self.run_exiv2("-M", "set Exif.Canon.OwnerName " + canary, target)
        self.run_exiv2("-M", "del Exif.Canon.OwnerName", target)
        self.assertNotIn(canary.encode(), target.read_bytes())
        self.run_exiv2("-M", "set Exif.Canon.OwnerName " + canary, target)
        self.run_exiv2("-M", "del Exif.Photo.MakerNote", target)
        self.assertNotIn(canary.encode(), target.read_bytes())
        self.assertFalse(list(self.root.glob(".exiv2-heif-*")))

    def test_read_only_fields_and_formats_do_not_change_source(self):
        for filename, key in [("Canon.HIF", "Exif.Canon.AFNumPoints"),
                              ("Canon-R6-pruned.CR3", "Exif.Image.Artist"),
                              ("Reagan.jxl", "Exif.Image.Artist"),
                              ("avif_exif_xmp.avif", "Exif.Image.Artist")]:
            with self.subTest(filename=filename):
                target = self.copy(filename)
                before = hashlib.sha256(target.read_bytes()).digest()
                self.run_exiv2("-M", "set " + key + " 123", target, success=False)
                self.assertEqual(hashlib.sha256(target.read_bytes()).digest(), before)
                self.assertFalse(list(self.root.glob(".exiv2-heif-*")))

    @unittest.skipUnless(os.name == "posix" and getattr(os, "geteuid", lambda: 0)() != 0,
                         "requires an unprivileged POSIX process")
    def test_staging_and_destination_open_failures_preserve_source(self):
        target = self.copy("Stonehenge.heic")
        before = target.read_bytes()
        self.root.chmod(0o500)
        try:
            self.run_exiv2("-M", "set Exif.Image.Artist pending", target, success=False)
            self.assertEqual(target.read_bytes(), before)
        finally:
            self.root.chmod(0o700)
        target.chmod(0o400)
        try:
            self.run_exiv2("-M", "set Exif.Image.Artist pending", target, success=False)
            self.assertEqual(target.read_bytes(), before)
        finally:
            target.chmod(0o600)
        self.assertFalse(list(self.root.glob(".exiv2-heif-*")))

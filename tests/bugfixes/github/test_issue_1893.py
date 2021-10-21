# -*- coding: utf-8 -*-

import system_tests
import unittest
from tempfile import TemporaryDirectory
import shutil
import hashlib
import os

# test needs system_tests.BT.vv['enable_bmff']=1
bSkip=system_tests.BT.verbose_version().get('enable_bmff')!='1'

if bSkip:
    raise unittest.SkipTest('*** requires enable_bmff=1 ***')

file_basename = 'Canon-R6-pruned.CR3'
previews_expected = (
    ('Canon-R6-pruned-preview1.jpg', 'a182ef12ac883309b4dfc66b87eac1891286d3ae'),
    ('Canon-R6-pruned-preview2.jpg', '524a07f1797854e349ae140e2682ba37147fa6b2')
)

class issue_1893_cr3_preview(metaclass=system_tests.CaseMeta):
    """
    Check that THMB and PRVW images are extracted from Canon CR3 files
    """
    url = "https://github.com/Exiv2/exiv2/issues/1893"
    filename = "$data_path/" + file_basename
    commands=[] # see setUp()

    if bSkip:
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        retval = [ 0, 0]
        stderr = [ "",""]
        stdin  = [ "", ""]
        stdout = ["""Preview 1: image/jpeg, 160x120 pixels, 16005 bytes
Preview 2: image/jpeg, 1620x1080 pixels, 389450 bytes
""", ""]

    def post_tests_hook(self):
        if self.commands:
            for j, sha1 in previews_expected:
                p = os.path.join(self.preview_image_tmp_dir.name, j)
                self.assertTrue(os.path.isfile(p))
                h = hashlib.sha1(open(p, 'rb').read()).hexdigest()
                self.assertEqual(h, sha1)

    def setUp(self):
        if bSkip:
            return
        # Avoid polluting the test data directory with extracted previews
        self.preview_image_tmp_dir = TemporaryDirectory()
        shutil.copy(self.expand_variables(self.filename),
                    self.preview_image_tmp_dir.name)
        p = os.path.join(
            self.preview_image_tmp_dir.name,
            file_basename)
        self.commands = [
            self.expand_variables("$exiv2 -pp ") + p,
            self.expand_variables("$exiv2 -ep ") + p
            ]

# -*- coding: utf-8 -*-

import os
import shutil

from system_tests import CaseMeta, path

# Regression tests for https://github.com/Exiv2/exiv2/issues/9510:
# out-of-range GPS values were cast from double to int, which is undefined
# behaviour and wrote garbage into the converted metadata.


class OutOfRangeGPSCoordExifToXmp(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9510"

    original_file = path("$data_path/exiv2-empty.jpg")
    filename = path("$tmp_path/issue_9510_coord.jpg")
    xmp_file = path("$tmp_path/issue_9510_coord.xmp")

    commands = ["$exiv2 -m- $filename", "$exiv2 -eX $filename"]
    stdin = [
        """set Exif.GPSInfo.GPSLatitude 2147483647/1 2147483647/1 0/1
set Exif.GPSInfo.GPSLatitudeRef N
""",
        None,
    ]
    stdout = ["", ""]
    stderr = ["", "Warning: Failed to convert Exif.GPSInfo.GPSLatitude to Xmp.exif.GPSLatitude\n"]
    retval = [0, 0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)
        if os.path.exists(self.xmp_file):
            os.remove(self.xmp_file)

    def tearDown(self):
        os.remove(self.filename)
        os.remove(self.xmp_file)


class OutOfRangeGPSTimeStampExifToXmp(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9510"

    original_file = path("$data_path/exiv2-empty.jpg")
    filename = path("$tmp_path/issue_9510_time.jpg")
    xmp_file = path("$tmp_path/issue_9510_time.xmp")

    commands = ["$exiv2 -m- $filename", "$exiv2 -eX $filename"]
    stdin = [
        """set Exif.GPSInfo.GPSTimeStamp 2147483647/1 2147483647/1 0/1
set Exif.GPSInfo.GPSDateStamp 2021:01:02
""",
        None,
    ]
    stdout = ["", ""]
    stderr = ["", "Warning: Failed to convert Exif.GPSInfo.GPSTimeStamp to Xmp.exif.GPSTimeStamp\n"]
    retval = [0, 0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)
        if os.path.exists(self.xmp_file):
            os.remove(self.xmp_file)

    def tearDown(self):
        os.remove(self.filename)
        os.remove(self.xmp_file)


class OutOfRangeGPSCoordXmpToExif(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9510"

    original_file = path("$data_path/exiv2-empty.jpg")
    filename = path("$tmp_path/issue_9510_xmp.jpg")
    xmp_file = path("$tmp_path/issue_9510_xmp.xmp")

    commands = ["$exiv2 -iX $filename"]
    stdout = [""]
    stderr = ["Warning: Failed to convert Xmp.exif.GPSLatitude to Exif.GPSInfo.GPSLatitude\n"]
    retval = [0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)
        with open(self.xmp_file, "w") as f:
            f.write(
                """<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about="" xmlns:exif="http://ns.adobe.com/exif/1.0/">
   <exif:GPSLatitude>0,3000000000N</exif:GPSLatitude>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>
"""
            )

    def tearDown(self):
        os.remove(self.filename)
        os.remove(self.xmp_file)

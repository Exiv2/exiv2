# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class TestVerboseExtractXmpSidecarToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose extracting XMP sidecar to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    filename = path("$data_path/issue_1934_poc4.jpg")
    commands = ["$exiv2 --verbose --extract X- $filename"]

    stdout = [
        """<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:exif="http://ns.adobe.com/exif/1.0/">
   <dc:subject>
    <rdf:Bag>
     <rdf:li>Value 1</rdf:li>
    </rdf:Bag>
   </dc:subject>
   <exif:UserComment>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">Value 1</rdf:li>
    </rdf:Alt>
   </exif:UserComment>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""
    ]

    stderr = [""]
    retval = [0]


class TestVerboseModifyRegAddExtractXmpSidecarToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose with modify register and add when extracting XMP sidecar to stdout'
    bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    filename = path("$data_path/issue_1934_poc4.jpg")
    commands = [
        """$exiv2 --verbose --Modify "reg TempGroup tempgroup/" --Modify "add Xmp.TempGroup.val Value 1" --extract X- $filename"""
    ]

    stdout = [
        """<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:TempGroup="tempgroup/"
    xmlns:exif="http://ns.adobe.com/exif/1.0/"
   TempGroup:val="Value 1">
   <dc:subject>
    <rdf:Bag>
     <rdf:li>Value 1</rdf:li>
    </rdf:Bag>
   </dc:subject>
   <exif:UserComment>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">Value 1</rdf:li>
    </rdf:Alt>
   </exif:UserComment>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""
    ]

    stderr = [""]
    retval = [0]


class TestVerboseModifySetExtractXmpSidecarToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose modify set when extracting XMP sidecar to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    filename = path("$data_path/issue_1934_poc4.jpg")
    commands = [
        """$exiv2 --verbose --Modify "set Xmp.dc.subject Value 2" --extract X- $filename"""
    ]

    stdout = [
        """<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:exif="http://ns.adobe.com/exif/1.0/">
   <dc:subject>
    <rdf:Bag>
     <rdf:li>Value 1</rdf:li>
     <rdf:li>Value 2</rdf:li>
    </rdf:Bag>
   </dc:subject>
   <exif:UserComment>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">Value 1</rdf:li>
    </rdf:Alt>
   </exif:UserComment>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""
    ]

    stderr = [""]
    retval = [0]


class TestVerboseModifyDelExtractXmpSidecarToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose modify delete when extracting XMP sidecar to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    filename = path("$data_path/issue_1934_poc4.jpg")
    commands = [
        """$exiv2 --verbose --Modify "del Xmp.dc.subject" --extract X- $filename"""
    ]

    stdout = [
        """<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:exif="http://ns.adobe.com/exif/1.0/">
   <exif:UserComment>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">Value 1</rdf:li>
    </rdf:Alt>
   </exif:UserComment>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""
    ]

    stderr = [""]
    retval = [0]


class TestVerboseExtractRawMetadataToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose extracting raw metadata to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    encodings = [bytes]

    def setUp(self):
        self.stdout = [bytes(open(self.expand_variables("$filename_ref"), "rb").read())]

    filename = path("$data_path/issue_1934_poc4.jpg")
    filename_ref = path("$data_path/issue_1934_poc4_ref.exv")

    commands = ["$exiv2 --verbose --extract XXeix- $filename"]

    stderr = [bytes([])]
    retval = [0]


class TestVerboseExtractThumbnailToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose extracting the thumbnail to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    encodings = [bytes]

    def setUp(self):
        self.stdout = [bytes(open(self.expand_variables("$filename_ref"), "rb").read())]

    filename = path("$data_path/issue_1934_poc4.jpg")
    filename_ref = path("$data_path/issue_1934_poc4-thumb_ref.jpg")

    commands = ["$exiv2 --verbose --extract t- $filename"]

    stderr = [bytes([])]
    retval = [0]


class TestVerboseExtractICCProfileToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose extracting the ICC profile to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    encodings = [bytes]

    def setUp(self):
        self.stdout = [bytes(open(self.expand_variables("$filename_ref"), "rb").read())]

    filename = path("$data_path/issue_1934_poc4.jpg")
    filename_ref = path("$data_path/issue_1934_poc4_ref.icc")

    commands = ["$exiv2 --verbose --extract C- $filename"]

    stderr = [bytes([])]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors


class TestVerboseExtractCommentToStdout(metaclass=CaseMeta):
    """
    Regression test for 'verbose extracting the comment to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    encodings = [bytes]

    def setUp(self):
        self.stdout = [bytes(open(self.expand_variables("$filename_ref"), "rb").read())]

    filename = path("$data_path/issue_1934_poc4.jpg")
    filename_ref = path("$data_path/issue_1934_poc4_comment_ref.txt")

    commands = ["$exiv2 --verbose --extract c- $filename"]

    stderr = [bytes([])]
    retval = [0]

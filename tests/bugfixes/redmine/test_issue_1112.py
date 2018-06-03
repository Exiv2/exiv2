# -*- coding: utf-8 -*-

import system_tests

@system_tests.CopyFiles("$data_path/exiv2-bug1112.xmp")
class CheckXmpTimeZoneInformation(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1112"

    filename = system_tests.path("$data_path/exiv2-bug1112_copy.xmp")
    commands = [ """$exiv2 -M "del Xmp.dc.title" $filename""" ]

    stdout = [ "" ]
    stderr = [""]
    retval = [0]

    xmp_packet = """<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
   xmp:CreateDate="2012-02-01T16:28:00+02:00"/>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""

    def post_tests_hook(self):
        with open(self.filename, "r", encoding='utf-8') as xmp_file:
            self.assertMultiLineEqual(self.xmp_packet, xmp_file.read(-1))

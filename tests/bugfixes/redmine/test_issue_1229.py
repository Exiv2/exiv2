# -*- coding: utf-8 -*-

import system_tests


@system_tests.DeleteFiles("$filename_xmp")
class CheckXmpData(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1229"
    filename = "$data_path/exiv2-bug1229.jpg"
    filename_xmp = "$data_path/exiv2-bug1229.xmp"

    commands = ["""$exiv2 -eX $filename"""]

    stdout = [""]
    stderr = [""]
    retval = [0]

    def post_tests_hook(self):
        with open(self.filename_xmp, "r", encoding='utf-8') as xmp:
            content = xmp.read(-1)

        expected =  """<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:xmpMM="http://ns.adobe.com/xap/1.0/mm/"
   xmpMM:InstanceID="uuid:e61b1e9d-b8e4-6a17-a443-74b51849baaf">
   <dc:subject>
    <rdf:Bag>
     <rdf:li>Celebs: JEnna Marbles</rdf:li>
    </rdf:Bag>
   </dc:subject>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""

        self.assertMultiLineEqual(expected, content)

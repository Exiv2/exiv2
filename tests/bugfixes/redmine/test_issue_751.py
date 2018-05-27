# -*- coding: utf-8 -*-

from system_tests import DeleteFiles, CopyFiles, CaseMeta, path


@DeleteFiles("$xmpname")
@CopyFiles("$data_path/exiv2-empty.jpg")
class AdobeXmpNamespace(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/751"

    filename = path("$data_path/exiv2-empty_copy.jpg")
    xmpname = path("$data_path/exiv2-empty_copy.xmp")

    commands = [
        """$exiv2 -v -M"reg imageapp orig/" -M "set Xmp.imageapp.uuid abcd" $filename""",
        "$exiv2 -f -eX $filename",
        "$cat $xmpname",
        """$exiv2 -v -M"reg imageapp dest/" -M "set Xmp.imageapp.uuid abcd" $filename""",
        "$exiv2 -f -eX $filename",
        "$cat $xmpname",
    ]

    stdout = [
        """File 1/1: $filename
Reg imageapp="orig/"
Set Xmp.imageapp.uuid "abcd" (XmpText)
""",
        "",
        """<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:imageapp="orig/"
   imageapp:uuid="abcd"/>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>""",
        """File 1/1: $filename
Reg imageapp="dest/"
Set Xmp.imageapp.uuid "abcd" (XmpText)
""",
        "",
        """<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:imageapp="orig/"
   imageapp:uuid="abcd"/>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""

    ]
    stderr = [
        "",
        "",
        "",
        """Warning: Updating namespace URI for imageapp from orig/ to dest/
""",
        """Warning: Updating namespace URI for imageapp from dest/ to orig/
""",
        ""
    ]
    retval = [0] * 6

# -*- coding: utf-8 -*-

import system_tests
import os


@system_tests.DeleteFiles("$xmpfile")
@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class WrongXmpTypeForNestedXmpKeys(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/$num"

    num = 799
    cmdfile = os.path.join("$data_path", "bug$num.cmd")

    filename_common = os.path.join("$data_path", "exiv2-empty_copy")
    filename = "$filename_common.jpg"
    xmpfile = "$filename_common.xmp"

    commands = [
        "$exiv2 -v -m $cmdfile $filename",
        "$exiv2 -v -pa $filename",
        "$exiv2 -f -eX $filename",
        "$cat $xmpfile",
    ]

    stdout = [
        """File 1/1: $filename
Set Xmp.MP.RegionInfo/MPRI:Regions "" (XmpBag)
Set Xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:Rectangle "0.11, 0.22, 0.33, 0.44" (XmpText)
Set Xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:PersonDisplayName "Baby Gnu" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions/stDim:w "1600" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions/stDim:h "800" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions/stDim:unit "pixel" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList "" (XmpBag)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Name "Baby Gnu" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Type "Face" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:x "0.275312" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:y "0.3775" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:w "0.164375" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:h "0.28125" (XmpText)
Set Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:unit "normalized" (XmpText)
""",
        """File 1/1: $filename
Xmp.MP.RegionInfo                            XmpText     0  type="Struct"
Xmp.MP.RegionInfo/MPRI:Regions               XmpText     0  type="Bag"
Xmp.MP.RegionInfo/MPRI:Regions[1]            XmpText     0  type="Struct"
Xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:Rectangle XmpText    22  0.11, 0.22, 0.33, 0.44
Xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:PersonDisplayName XmpText     8  Baby Gnu
Xmp.mwg-rs.Regions                           XmpText     0  type="Struct"
Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions XmpText     0  type="Struct"
Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions/stDim:w XmpText     4  1600
Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions/stDim:h XmpText     3  800
Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions/stDim:unit XmpText     5  pixel
Xmp.mwg-rs.Regions/mwg-rs:RegionList         XmpText     0  type="Bag"
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]      XmpText     0  type="Struct"
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Name XmpText     8  Baby Gnu
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Type XmpText     4  Face
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area XmpText     0  type="Struct"
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:x XmpText     8  0.275312
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:y XmpText     6  0.3775
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:w XmpText     8  0.164375
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:h XmpText     7  0.28125
Xmp.mwg-rs.Regions/mwg-rs:RegionList[1]/mwg-rs:Area/stArea:unit XmpText    10  normalized
""",
        "",
"""<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:MP="http://ns.microsoft.com/photo/1.2/"
    xmlns:MPRI="http://ns.microsoft.com/photo/1.2/t/RegionInfo#"
    xmlns:MPReg="http://ns.microsoft.com/photo/1.2/t/Region#"
    xmlns:mwg-rs="http://www.metadataworkinggroup.com/schemas/regions/"
    xmlns:stDim="http://ns.adobe.com/xap/1.0/sType/Dimensions#"
    xmlns:stArea="http://ns.adobe.com/xmp/sType/Area#">
   <MP:RegionInfo rdf:parseType="Resource">
    <MPRI:Regions>
     <rdf:Bag>
      <rdf:li
       MPReg:Rectangle="0.11, 0.22, 0.33, 0.44"
       MPReg:PersonDisplayName="Baby Gnu"/>
     </rdf:Bag>
    </MPRI:Regions>
   </MP:RegionInfo>
   <mwg-rs:Regions rdf:parseType="Resource">
    <mwg-rs:AppliedToDimensions
     stDim:w="1600"
     stDim:h="800"
     stDim:unit="pixel"/>
    <mwg-rs:RegionList>
     <rdf:Bag>
      <rdf:li>
       <rdf:Description
        mwg-rs:Name="Baby Gnu"
        mwg-rs:Type="Face">
       <mwg-rs:Area
        stArea:x="0.275312"
        stArea:y="0.3775"
        stArea:w="0.164375"
        stArea:h="0.28125"
        stArea:unit="normalized"/>
       </rdf:Description>
      </rdf:li>
     </rdf:Bag>
    </mwg-rs:RegionList>
   </mwg-rs:Regions>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>"""
    ]

    stderr = [
        "",
        """$filename: No Exif data found in the file
$filename: No IPTC data found in the file
""",
        "",
        ""
    ]
    retval = [0] * 4

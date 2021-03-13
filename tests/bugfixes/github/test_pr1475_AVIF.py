# -*- coding: utf-8 -*-

import system_tests

# test needs system_tests.BT.vv.enable_bmff=1
vv = system_tests.BT.verbose_version()
bSkip = vv.get('enable_bmff') != '1'

class pr_1475_avif_avif(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/avif.avif"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2 -pa $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ,"$exiv2 -pC $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.ExifTag                           Long        1  26
Exif.Photo.DateTimeOriginal                  Ascii      20  2018:08:15 11:37:35
Exif.Photo.SubSecTimeOriginal                Ascii       3  59
""","""Exiv2::BmffImage::boxHandler: ftyp        0->32 brand: avif
Exiv2::BmffImage::boxHandler: meta       32->304 
  Exiv2::BmffImage::boxHandler: hdlr       44->33 
  Exiv2::BmffImage::boxHandler: pitm       77->14 
  Exiv2::BmffImage::boxHandler: iinf       91->56 
    Exiv2::BmffImage::boxHandler: infe      105->21 ID =   1 av01 
    Exiv2::BmffImage::boxHandler: infe      126->21 ID =   2 Exif  *** Exif ***
  Exiv2::BmffImage::boxHandler: iloc      147->44 
       155 |       14 |   ID |    1 |    408,218726
       169 |       14 |   ID |    2 | 219134,    82
  Exiv2::BmffImage::boxHandler: iprp      191->119 
    Exiv2::BmffImage::boxHandler: ipco      199->88 
      Exiv2::BmffImage::boxHandler: ispe      207->20 pixelWidth_, pixelHeight_ = 1920, 1080
      Exiv2::BmffImage::boxHandler: colr      227->19 
      Exiv2::BmffImage::boxHandler: av1C      246->25 
      Exiv2::BmffImage::boxHandler: pixi      271->16 
    Exiv2::BmffImage::boxHandler: ipma      287->23 
  Exiv2::BmffImage::boxHandler: iref      310->26 
Exiv2::BMFF Exif: ID = 2 from,length = 219134,82
Exiv2::BmffImage::boxHandler: free      336->64 
Exiv2::BmffImage::boxHandler: mdat      400->218816 
""","",""]

class pr_1475_exif_xmp_avif(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/avif_exif_xmp.avif"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2 -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ,"$exiv2 -pC $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.DateTime                          Ascii      20  2021:02:13 21:19:50
Xmp.iptcExt.DigitalSourceType                XmpText    61  http://cv.iptc.org/newscodes/digitalsourcetype/digitalCapture
Xmp.iptcExt.LocationCreated                  XmpBag      0  
Xmp.iptcExt.LocationShown                    XmpBag      0  
Xmp.iptcExt.ArtworkOrObject                  XmpBag      0  
Xmp.iptcExt.RegistryId                       XmpBag      0  
Xmp.xmpMM.DocumentID                         XmpText    52  gimp:docid:gimp:8cf1b29a-9b12-4dec-9930-f2c608b298de
Xmp.xmpMM.InstanceID                         XmpText    44  xmp.iid:33e259b4-7237-4b2a-87bf-4609986012a8
Xmp.xmpMM.OriginalDocumentID                 XmpText    44  xmp.did:02dfa8e8-ce11-4de7-971d-566a5bba5edb
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[1]/stEvt:changed           XmpText     9  /metadata
Xmp.xmpMM.History[1]/stEvt:instanceID        XmpText    44  xmp.iid:446f9de2-f964-4680-8461-ddf05bfcc53e
Xmp.xmpMM.History[1]/stEvt:softwareAgent     XmpText    19  GIMP 2.99.5 (Linux)
Xmp.xmpMM.History[1]/stEvt:when              XmpText    25  2021-02-13T21:19:04+01:00
Xmp.xmpMM.History[2]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[2]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[2]/stEvt:changed           XmpText     1  /
Xmp.xmpMM.History[2]/stEvt:instanceID        XmpText    44  xmp.iid:017dc808-2418-4f70-99f3-aa05905adb88
Xmp.xmpMM.History[2]/stEvt:softwareAgent     XmpText    19  GIMP 2.99.5 (Linux)
Xmp.xmpMM.History[2]/stEvt:when              XmpText    25  2021-02-13T21:20:14+01:00
Xmp.plus.ModelReleaseStatus                  XmpText    38  None
Xmp.plus.ImageSupplier                       XmpSeq      0  
Xmp.plus.ImageCreator                        XmpSeq      0  
Xmp.plus.CopyrightOwner                      XmpSeq      0  
Xmp.plus.Licensor                            XmpSeq      0  
Xmp.GIMP.API                                 XmpText     3  3.0
Xmp.GIMP.Platform                            XmpText     5  Linux
Xmp.GIMP.TimeStamp                           XmpText    16  1613247614397805
Xmp.GIMP.Version                             XmpText     6  2.99.5
Xmp.dc.Format                                XmpText    10  image/avif
Xmp.dc.creator                               XmpSeq      1  type="Seq" Developer
Xmp.dc.description                           LangAlt     1  lang="x-default" This is a testfile
Xmp.dc.rights                                LangAlt     1  lang="x-default" It is forbidden to use for evil purposes
Xmp.dc.title                                 LangAlt     1  lang="x-default" Personal photo
Xmp.photoshop.AuthorsPosition                XmpText    18  Computer Scientist
Xmp.xmp.CreatorTool                          XmpText     4  GIMP
Xmp.xmp.Rating                               XmpText     1  5
Xmp.xmpRights.Marked                         XmpText     4  True
""","""Exiv2::BmffImage::boxHandler: ftyp        0->32 brand: avif
Exiv2::BmffImage::boxHandler: meta       32->379 
  Exiv2::BmffImage::boxHandler: hdlr       44->40 
  Exiv2::BmffImage::boxHandler: pitm       84->14 
  Exiv2::BmffImage::boxHandler: iloc       98->58 
       106 |       14 |   ID |    1 |   5418,  5445
       120 |       14 |   ID |    2 |    419,   316
       134 |       14 |   ID |    3 |    735,  4683
  Exiv2::BmffImage::boxHandler: iinf      156->109 
    Exiv2::BmffImage::boxHandler: infe      170->26 ID =   1 av01Color 
    Exiv2::BmffImage::boxHandler: infe      196->25 ID =   2 ExifExif  *** Exif ***
    Exiv2::BmffImage::boxHandler: infe      221->44 ID =   3 mimeXMP  *** XMP ***
  Exiv2::BmffImage::boxHandler: iref      265->40 
  Exiv2::BmffImage::boxHandler: iprp      305->106 
    Exiv2::BmffImage::boxHandler: ipco      313->75 
      Exiv2::BmffImage::boxHandler: ispe      321->20 pixelWidth_, pixelHeight_ = 120, 120
      Exiv2::BmffImage::boxHandler: pixi      341->16 
      Exiv2::BmffImage::boxHandler: av1C      357->12 
      Exiv2::BmffImage::boxHandler: colr      369->19 
    Exiv2::BmffImage::boxHandler: ipma      388->23 
Exiv2::BMFF Exif: ID = 2 from,length = 419,316
Exiv2::BMFF XMP: ID = 3 from,length = 735,4683
Exiv2::BmffImage::boxHandler: mdat      411->10452 
""","""<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:iptcExt="http://iptc.org/std/Iptc4xmpExt/2008-02-29/"
    xmlns:xmpMM="http://ns.adobe.com/xap/1.0/mm/"
    xmlns:stEvt="http://ns.adobe.com/xap/1.0/sType/ResourceEvent#"
    xmlns:plus="http://ns.useplus.org/ldf/xmp/1.0/"
    xmlns:GIMP="http://www.gimp.org/xmp/"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:photoshop="http://ns.adobe.com/photoshop/1.0/"
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
    xmlns:xmpRights="http://ns.adobe.com/xap/1.0/rights/"
   iptcExt:DigitalSourceType="http://cv.iptc.org/newscodes/digitalsourcetype/digitalCapture"
   xmpMM:DocumentID="gimp:docid:gimp:8cf1b29a-9b12-4dec-9930-f2c608b298de"
   xmpMM:InstanceID="xmp.iid:33e259b4-7237-4b2a-87bf-4609986012a8"
   xmpMM:OriginalDocumentID="xmp.did:02dfa8e8-ce11-4de7-971d-566a5bba5edb"
   plus:ModelReleaseStatus="http://ns.useplus.org/ldf/vocab/MR-NON"
   GIMP:API="3.0"
   GIMP:Platform="Linux"
   GIMP:TimeStamp="1613247614397805"
   GIMP:Version="2.99.5"
   dc:Format="image/avif"
   photoshop:AuthorsPosition="Computer Scientist"
   xmp:CreatorTool="GIMP"
   xmp:Rating="5"
   xmpRights:Marked="True">
   <iptcExt:LocationCreated>
    <rdf:Bag/>
   </iptcExt:LocationCreated>
   <iptcExt:LocationShown>
    <rdf:Bag/>
   </iptcExt:LocationShown>
   <iptcExt:ArtworkOrObject>
    <rdf:Bag/>
   </iptcExt:ArtworkOrObject>
   <iptcExt:RegistryId>
    <rdf:Bag/>
   </iptcExt:RegistryId>
   <xmpMM:History>
    <rdf:Seq>
     <rdf:li
      stEvt:action="saved"
      stEvt:changed="/metadata"
      stEvt:instanceID="xmp.iid:446f9de2-f964-4680-8461-ddf05bfcc53e"
      stEvt:softwareAgent="GIMP 2.99.5 (Linux)"
      stEvt:when="2021-02-13T21:19:04+01:00"/>
     <rdf:li
      stEvt:action="saved"
      stEvt:changed="/"
      stEvt:instanceID="xmp.iid:017dc808-2418-4f70-99f3-aa05905adb88"
      stEvt:softwareAgent="GIMP 2.99.5 (Linux)"
      stEvt:when="2021-02-13T21:20:14+01:00"/>
    </rdf:Seq>
   </xmpMM:History>
   <plus:ImageSupplier>
    <rdf:Seq/>
   </plus:ImageSupplier>
   <plus:ImageCreator>
    <rdf:Seq/>
   </plus:ImageCreator>
   <plus:CopyrightOwner>
    <rdf:Seq/>
   </plus:CopyrightOwner>
   <plus:Licensor>
    <rdf:Seq/>
   </plus:Licensor>
   <dc:creator>
    <rdf:Seq>
     <rdf:li>type="Seq" Developer</rdf:li>
    </rdf:Seq>
   </dc:creator>
   <dc:description>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">This is a testfile</rdf:li>
    </rdf:Alt>
   </dc:description>
   <dc:rights>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">It is forbidden to use for evil purposes</rdf:li>
    </rdf:Alt>
   </dc:rights>
   <dc:title>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">Personal photo</rdf:li>
    </rdf:Alt>
   </dc:title>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",""]

class pr_1475_metadata2_avif(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/avif_metadata2.avif"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2 -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ,"$exiv2 -pC $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.DateTime                          Ascii      20  2021:02:13 21:25:32
Xmp.iptcExt.DigitalSourceType                XmpText    61  http://cv.iptc.org/newscodes/digitalsourcetype/digitalCapture
Xmp.iptcExt.LocationCreated                  XmpBag      0  
Xmp.iptcExt.LocationShown                    XmpBag      0  
Xmp.iptcExt.ArtworkOrObject                  XmpBag      0  
Xmp.iptcExt.RegistryId                       XmpBag      0  
Xmp.xmpMM.DocumentID                         XmpText    52  gimp:docid:gimp:e38231ee-2d28-42e6-b17e-6fa80c9fd85b
Xmp.xmpMM.InstanceID                         XmpText    44  xmp.iid:34774f89-42e3-4411-a691-f8cdb5b11def
Xmp.xmpMM.OriginalDocumentID                 XmpText    44  xmp.did:e2129b55-045f-4085-bbd2-e11d11edfa0c
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[1]/stEvt:changed           XmpText     9  /metadata
Xmp.xmpMM.History[1]/stEvt:instanceID        XmpText    44  xmp.iid:928b36e2-0239-4616-847a-eff7a31816e3
Xmp.xmpMM.History[1]/stEvt:softwareAgent     XmpText    19  GIMP 2.99.5 (Linux)
Xmp.xmpMM.History[1]/stEvt:when              XmpText    25  2021-02-13T21:24:26+01:00
Xmp.xmpMM.History[2]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[2]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[2]/stEvt:changed           XmpText     1  /
Xmp.xmpMM.History[2]/stEvt:instanceID        XmpText    44  xmp.iid:23f267c5-c40a-45c8-947d-11313b4881c1
Xmp.xmpMM.History[2]/stEvt:softwareAgent     XmpText    19  GIMP 2.99.5 (Linux)
Xmp.xmpMM.History[2]/stEvt:when              XmpText    25  2021-02-13T21:25:41+01:00
Xmp.plus.ModelReleaseStatus                  XmpText    38  None
Xmp.plus.ImageSupplier                       XmpSeq      0  
Xmp.plus.ImageCreator                        XmpSeq      0  
Xmp.plus.CopyrightOwner                      XmpSeq      0  
Xmp.plus.Licensor                            XmpSeq      0  
Xmp.GIMP.API                                 XmpText     3  3.0
Xmp.GIMP.Platform                            XmpText     5  Linux
Xmp.GIMP.TimeStamp                           XmpText    16  1613247941462908
Xmp.GIMP.Version                             XmpText     6  2.99.5
Xmp.dc.Format                                XmpText    10  image/avif
Xmp.dc.creator                               XmpSeq      1  type="Seq" me
Xmp.dc.description                           LangAlt     1  lang="x-default" bla bla bla
Xmp.xmp.CreatorTool                          XmpText     4  GIMP
""","""Exiv2::BmffImage::boxHandler: ftyp        0->24 brand: avif
Exiv2::BmffImage::boxHandler: meta       24->356 
  Exiv2::BmffImage::boxHandler: hdlr       36->33 
  Exiv2::BmffImage::boxHandler: pitm       69->14 
  Exiv2::BmffImage::boxHandler: iloc       83->70 
        91 |       18 |   ID |    1 |    388,  4730
       109 |       18 |   ID |    2 |   5118,   412
       127 |       18 |   ID |    3 |   5530,  4254
  Exiv2::BmffImage::boxHandler: iinf      153->98 
    Exiv2::BmffImage::boxHandler: infe      167->21 ID =   1 av01 
    Exiv2::BmffImage::boxHandler: infe      188->21 ID =   2 Exif  *** Exif ***
    Exiv2::BmffImage::boxHandler: infe      209->42 ID =   3 mime  *** XMP ***
  Exiv2::BmffImage::boxHandler: iprp      251->89 
    Exiv2::BmffImage::boxHandler: ipco      259->59 
      Exiv2::BmffImage::boxHandler: colr      267->19 
      Exiv2::BmffImage::boxHandler: av1C      286->12 
      Exiv2::BmffImage::boxHandler: ispe      298->20 pixelWidth_, pixelHeight_ = 120, 120
    Exiv2::BmffImage::boxHandler: ipma      318->22 
  Exiv2::BmffImage::boxHandler: iref      340->40 
Exiv2::BMFF Exif: ID = 2 from,length = 5118,412
Exiv2::BMFF XMP: ID = 3 from,length = 5530,4254
Exiv2::BmffImage::boxHandler: mdat      380->9404 
""","""<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:iptcExt="http://iptc.org/std/Iptc4xmpExt/2008-02-29/"
    xmlns:xmpMM="http://ns.adobe.com/xap/1.0/mm/"
    xmlns:stEvt="http://ns.adobe.com/xap/1.0/sType/ResourceEvent#"
    xmlns:plus="http://ns.useplus.org/ldf/xmp/1.0/"
    xmlns:GIMP="http://www.gimp.org/xmp/"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
   iptcExt:DigitalSourceType="http://cv.iptc.org/newscodes/digitalsourcetype/digitalCapture"
   xmpMM:DocumentID="gimp:docid:gimp:e38231ee-2d28-42e6-b17e-6fa80c9fd85b"
   xmpMM:InstanceID="xmp.iid:34774f89-42e3-4411-a691-f8cdb5b11def"
   xmpMM:OriginalDocumentID="xmp.did:e2129b55-045f-4085-bbd2-e11d11edfa0c"
   plus:ModelReleaseStatus="http://ns.useplus.org/ldf/vocab/MR-NON"
   GIMP:API="3.0"
   GIMP:Platform="Linux"
   GIMP:TimeStamp="1613247941462908"
   GIMP:Version="2.99.5"
   dc:Format="image/avif"
   xmp:CreatorTool="GIMP">
   <iptcExt:LocationCreated>
    <rdf:Bag/>
   </iptcExt:LocationCreated>
   <iptcExt:LocationShown>
    <rdf:Bag/>
   </iptcExt:LocationShown>
   <iptcExt:ArtworkOrObject>
    <rdf:Bag/>
   </iptcExt:ArtworkOrObject>
   <iptcExt:RegistryId>
    <rdf:Bag/>
   </iptcExt:RegistryId>
   <xmpMM:History>
    <rdf:Seq>
     <rdf:li
      stEvt:action="saved"
      stEvt:changed="/metadata"
      stEvt:instanceID="xmp.iid:928b36e2-0239-4616-847a-eff7a31816e3"
      stEvt:softwareAgent="GIMP 2.99.5 (Linux)"
      stEvt:when="2021-02-13T21:24:26+01:00"/>
     <rdf:li
      stEvt:action="saved"
      stEvt:changed="/"
      stEvt:instanceID="xmp.iid:23f267c5-c40a-45c8-947d-11313b4881c1"
      stEvt:softwareAgent="GIMP 2.99.5 (Linux)"
      stEvt:when="2021-02-13T21:25:41+01:00"/>
    </rdf:Seq>
   </xmpMM:History>
   <plus:ImageSupplier>
    <rdf:Seq/>
   </plus:ImageSupplier>
   <plus:ImageCreator>
    <rdf:Seq/>
   </plus:ImageCreator>
   <plus:CopyrightOwner>
    <rdf:Seq/>
   </plus:CopyrightOwner>
   <plus:Licensor>
    <rdf:Seq/>
   </plus:Licensor>
   <dc:creator>
    <rdf:Seq>
     <rdf:li>type="Seq" me</rdf:li>
    </rdf:Seq>
   </dc:creator>
   <dc:description>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">bla bla bla</rdf:li>
    </rdf:Alt>
   </dc:description>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",""]


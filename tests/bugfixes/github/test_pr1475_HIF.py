# -*- coding: utf-8 -*-

import system_tests

# test needs system_tests.BT.vv.enable_bmff=1
vv = system_tests.BT.verbose_version()
bSkip = vv.get('enable_bmff') != '1'

class pr_1475_Sony_hif(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/Sony.HIF"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2  -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ,"$exiv2 -pC $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.Make                              Ascii       5  SONY
Exif.Image.DateTime                          Ascii      20  2021:02:18 19:55:41
Exif.Photo.ExposureTime                      Rational    1  1/1000 s
Exif.Photo.ExposureProgram                   Short       1  Manual
Exif.Photo.RecommendedExposureIndex          Long        1  100
Exif.Photo.DateTimeOriginal                  Ascii      20  2021:02:18 19:55:41
Exif.Photo.DateTimeDigitized                 Ascii      20  2021:02:18 19:55:41
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.Flash                             Short       1  No, compulsory
Exif.Sony1.FlashExposureComp                 SRational   1  0 EV
Exif.Sony1.ExposureMode                      Short       1  Manual
Exif.Sony1.FlashLevel                        SShort      1  Normal
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ExposureMode                      Short       1  Manual
Xmp.xmp.Rating                               XmpText     1  0
""","""Exiv2::BmffImage::boxHandler: ftyp        0->40 brand: heix
Exiv2::BmffImage::boxHandler: meta       40->2081 
  Exiv2::BmffImage::boxHandler: hdlr       52->33 
  Exiv2::BmffImage::boxHandler: pitm       85->14 
  Exiv2::BmffImage::boxHandler: iinf       99->350 
    Exiv2::BmffImage::boxHandler: infe      113->21 ID =   1 hvc1 
    Exiv2::BmffImage::boxHandler: infe      134->21 ID =   2 hvc1 
    Exiv2::BmffImage::boxHandler: infe      155->21 ID =   3 hvc1 
    Exiv2::BmffImage::boxHandler: infe      176->21 ID =   4 hvc1 
    Exiv2::BmffImage::boxHandler: infe      197->21 ID =   5 hvc1 
    Exiv2::BmffImage::boxHandler: infe      218->21 ID =   6 hvc1 
    Exiv2::BmffImage::boxHandler: infe      239->21 ID =   7 hvc1 
    Exiv2::BmffImage::boxHandler: infe      260->21 ID =   8 hvc1 
    Exiv2::BmffImage::boxHandler: infe      281->21 ID =   9 hvc1 
    Exiv2::BmffImage::boxHandler: infe      302->21 ID =  10 grid 
    Exiv2::BmffImage::boxHandler: infe      323->21 ID =  11 hvc1 
    Exiv2::BmffImage::boxHandler: infe      344->21 ID =  12 hvc1 
    Exiv2::BmffImage::boxHandler: infe      365->21 ID =  13 jpeg 
    Exiv2::BmffImage::boxHandler: infe      386->21 ID =  14 Exif  *** Exif ***
    Exiv2::BmffImage::boxHandler: infe      407->42 ID =  15 mime  *** XMP ***
  Exiv2::BmffImage::boxHandler: iref      449->112 
  Exiv2::BmffImage::boxHandler: iprp      561->1288 
    Exiv2::BmffImage::boxHandler: ipco      569->1184 
      Exiv2::BmffImage::boxHandler: irot      577->9 
      Exiv2::BmffImage::boxHandler: colr      586->19 
      Exiv2::BmffImage::boxHandler: pixi      605->16 
      Exiv2::BmffImage::boxHandler: hvcC      621->345 
      Exiv2::BmffImage::boxHandler: ispe      966->20 pixelWidth_, pixelHeight_ = 2880, 1920
      Exiv2::BmffImage::boxHandler: ispe      986->20 pixelWidth_, pixelHeight_ = 8640, 5760
      Exiv2::BmffImage::boxHandler: hvcC     1006->344 
      Exiv2::BmffImage::boxHandler: ispe     1350->20 pixelWidth_, pixelHeight_ = 1616, 1080
      Exiv2::BmffImage::boxHandler: hvcC     1370->343 
      Exiv2::BmffImage::boxHandler: ispe     1713->20 pixelWidth_, pixelHeight_ = 320, 212
      Exiv2::BmffImage::boxHandler: ispe     1733->20 pixelWidth_, pixelHeight_ = 160, 120
    Exiv2::BmffImage::boxHandler: ipma     1753->96 
  Exiv2::BmffImage::boxHandler: idat     1849->16 
  Exiv2::BmffImage::boxHandler: iloc     1865->256 
      1873 |       16 |   ID |   15 |   4096, 57344
      1889 |       16 |   ID |   14 |  61440, 40960
      1905 |       16 |   ID |   13 | 102400,  4096
      1921 |       16 |   ID |   12 | 106496,  4096
      1937 |       16 |   ID |    1 | 110592,  4562
      1953 |       16 |   ID |    2 | 115154,  2802
      1969 |       16 |   ID |    3 | 117956,  4344
      1985 |       16 |   ID |    4 | 122300,  3772
      2001 |       16 |   ID |    5 | 126072,  2227
      2017 |       16 |   ID |    6 | 128299,  3668
      2033 |       16 |   ID |    7 | 131967,  4518
      2049 |       16 |   ID |    8 | 136485,  2840
      2065 |       16 |   ID |    9 | 139325,  8131
      2081 |       16 |   ID |   10 |      0,     8
      2097 |       16 |   ID |   11 | 147456,  4096
Exiv2::BMFF Exif: ID = 14 from,length = 61440,40960
Exiv2::BMFF XMP: ID = 15 from,length = 4096,57344
Exiv2::BmffImage::boxHandler: free     2121->1967 
Exiv2::BmffImage::boxHandler: mdat     4088->147464 
""","""<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
   xmp:Rating="0"/>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",""]

class pr_1475_Canon_hif(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/Canon.HIF"

    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdin=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2  -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ,"$exiv2 -pC $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.Make                              Ascii       6  Canon
Exif.Image.DateTime                          Ascii      20  2021:02:18 19:54:47
Exif.Photo.ExposureTime                      Rational    1  1/1000 s
Exif.Photo.ExposureProgram                   Short       1  Manual
Exif.Photo.RecommendedExposureIndex          Long        1  100
Exif.Photo.DateTimeOriginal                  Ascii      20  2021:02:18 19:54:47
Exif.Photo.DateTimeDigitized                 Ascii      20  2021:02:18 19:54:47
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.Flash                             Short       1  No flash
Exif.CanonCs.FlashMode                       Short       1  Off
Exif.CanonCs.ExposureProgram                 Short       1  Manual (M)
Exif.CanonCs.FlashActivity                   Short       1  Did not fire
Exif.CanonCs.FlashDetails                    Short       1  E-TTL
Exif.CanonCs.ManualFlashOutput               Short       1  n/a
Exif.CanonSi.FlashGuideNumber                Short       1  0
Exif.CanonSi.FlashBias                       Short       1  0 EV
Exif.CanonSi.AutoExposureBracketing          Short       1  Off
Exif.CanonFi.FlashExposureLock               SShort      1  Off
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ExposureMode                      Short       1  Manual
Xmp.xmp.Rating                               XmpText     1  0
""","""Exiv2::BmffImage::boxHandler: ftyp        0->32 brand: heix
Exiv2::BmffImage::boxHandler: meta       32->1163 
  Exiv2::BmffImage::boxHandler: hdlr       44->33 
  Exiv2::BmffImage::boxHandler: uuid       77->62  uuidName cano
    Exiv2::BmffImage::boxHandler: CNCV      101->38 
  Exiv2::BmffImage::boxHandler: dinf      139->36 
  Exiv2::BmffImage::boxHandler: pitm      175->14 
  Exiv2::BmffImage::boxHandler: iinf      189->203 
    Exiv2::BmffImage::boxHandler: infe      203->21 ID =   1 grid 
    Exiv2::BmffImage::boxHandler: infe      224->21 ID = 256 hvc1 
    Exiv2::BmffImage::boxHandler: infe      245->21 ID = 257 hvc1 
    Exiv2::BmffImage::boxHandler: infe      266->21 ID = 258 hvc1 
    Exiv2::BmffImage::boxHandler: infe      287->21 ID = 259 hvc1 
    Exiv2::BmffImage::boxHandler: infe      308->21 ID = 512 hvc1 
    Exiv2::BmffImage::boxHandler: infe      329->21 ID = 768 Exif  *** Exif ***
    Exiv2::BmffImage::boxHandler: infe      350->42 ID = 769 mime  *** XMP ***
  Exiv2::BmffImage::boxHandler: iref      392->74 
  Exiv2::BmffImage::boxHandler: iprp      466->569 
    Exiv2::BmffImage::boxHandler: ipco      474->507 
      Exiv2::BmffImage::boxHandler: hvcC      482->176 
      Exiv2::BmffImage::boxHandler: ispe      658->20 pixelWidth_, pixelHeight_ = 1216, 832
      Exiv2::BmffImage::boxHandler: colr      678->19 
      Exiv2::BmffImage::boxHandler: pixi      697->16 
      Exiv2::BmffImage::boxHandler: ispe      713->20 pixelWidth_, pixelHeight_ = 2400, 1600
      Exiv2::BmffImage::boxHandler: irot      733->9 
      Exiv2::BmffImage::boxHandler: hvcC      742->175 
      Exiv2::BmffImage::boxHandler: ispe      917->20 pixelWidth_, pixelHeight_ = 320, 214
      Exiv2::BmffImage::boxHandler: colr      937->19 
      Exiv2::BmffImage::boxHandler: pixi      956->16 
      Exiv2::BmffImage::boxHandler: irot      972->9 
    Exiv2::BmffImage::boxHandler: ipma      981->54 
  Exiv2::BmffImage::boxHandler: idat     1035->16 
  Exiv2::BmffImage::boxHandler: iloc     1051->144 
      1059 |       16 |   ID |    1 |      0,     8
      1075 |       16 |   ID |  256 |  46080,344284
      1091 |       16 |   ID |  257 | 390364,340989
      1107 |       16 |   ID |  258 | 731353,257177
      1123 |       16 |   ID |  259 | 988530,264862
      1139 |       16 |   ID |  512 |  32256, 10284
      1155 |       16 |   ID |  768 |   1536, 30463
      1171 |       16 |   ID |  769 |  43008,  3072
Exiv2::BMFF Exif: ID = 768 from,length = 1536,30463
Exiv2::BMFF XMP: ID = 769 from,length = 43008,3072
Exiv2::BmffImage::boxHandler: mdat     1195->1252197 
""","""<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
   xmp:Rating="0"/>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",""]


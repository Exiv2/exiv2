# -*- coding: utf-8 -*-

import itertools
import os.path

from system_tests import CaseMeta, path


def read_file(filename):
    with open(filename, 'r') as f:
        return f.read()


class AddMinusPSOption(metaclass=CaseMeta):

    cls_location = os.path.dirname(__file__)

    url = "http://dev.exiv2.org/issues/922"

    bug_jpg_file = path("$data_path/exiv2-bug922.jpg")
    IPTC_file = path("$data_path/iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg")
    files = [
        path("$data_path/{!s}".format(img))
        for img in "exiv2-bug922.png exiv2-bug922.tif exiv2-bug922a.jpg".split()
    ]

    png_bug_file = files[0]
    tif_bug_file = files[1]
    jpg_bug_file = files[2]

    commands = [
        "$exiv2 -pX $bug_jpg_file",
        "$exiv2 -pX $IPTC_file",
    ] + list(
        itertools.chain.from_iterable([
            "$exiv2 -pX " + fname,
            "$exiv2 -pS " + fname
        ] for fname in files)
    )

    stdout = [
        read_file(
            os.path.join(cls_location, "issue_922_exiv2_pX_bug922_jpg_output")
        ),
        """<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.1.1-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:xap="http://ns.adobe.com/xap/1.0/"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:photoshop="http://ns.adobe.com/photoshop/1.0/"
    xmlns:xapMM="http://ns.adobe.com/xap/1.0/mm/"
    xmlns:tiff="http://ns.adobe.com/tiff/1.0/"
    xmlns:exif="http://ns.adobe.com/exif/1.0/"
   xap:CreateDate="2009-06-03T17:02:22+02:00"
   xap:ModifyDate="2009-06-03T17:02:22+02:00"
   xap:MetadataDate="2009-06-03T17:02:22+02:00"
   xap:CreatorTool="Adobe Photoshop CS3 Macintosh"
   dc:format="image/jpeg"
   photoshop:ColorMode="3"
   photoshop:History=""
   xapMM:InstanceID="uuid:6E0DE597F251DE119368FCA8AE42D1CE"
   xapMM:DocumentID="uuid:6D0DE597F251DE119368FCA8AE42D1CE"
   tiff:Orientation="1"
   tiff:XResolution="720000/10000"
   tiff:YResolution="720000/10000"
   tiff:ResolutionUnit="2"
   tiff:NativeDigest="256,257,258,259,262,274,277,284,530,531,282,283,296,301,318,319,529,532,306,270,271,272,305,315,33432;A50D672D77BFA6B1794A470713D72F1B"
   exif:PixelXDimension="420"
   exif:PixelYDimension="300"
   exif:ColorSpace="-1"
   exif:NativeDigest="36864,40960,40961,37121,37122,40962,40963,37510,40964,36867,36868,33434,33437,34850,34852,34855,34856,37377,37378,37379,37380,37381,37382,37383,37384,37385,37386,37396,41483,41484,41486,41487,41488,41492,41493,41495,41728,41729,41730,41985,41986,41987,41988,41989,41990,41991,41992,41993,41994,41995,41996,42016,0,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,23,24,25,26,27,28,30;994BCC01E43CFBC321A79B1A803AAF9B">
   <xapMM:DerivedFrom rdf:parseType="Resource"/>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",
        """<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:dc="http://purl.org/dc/elements/1.1/">
   <dc:title>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">this is the title</rdf:li>
    </rdf:Alt>
   </dc:title>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",
        """STRUCTURE OF PNG FILE: $png_bug_file
 address | chunk |  length | data                           | checksum
       8 | IHDR  |      13 | ...@........                   | 0x7f775da4
      33 | zTXt  |    8769 | Raw profile type exif..x...[r. | 0x4a89d860
    8814 | zTXt  |     270 | Raw profile type iptc..x.=QKn. | 0x29f9e2d3
    9096 | iTXt  |    2524 | XML:com.adobe.xmp.....<?xpacke | 0x1df4a351
   11632 | iCCP  |    1404 | icc..x...i8........af\...w_3.. | 0x363e2409
   13048 | sBIT  |       3 | ...                            | 0xdbe14fe0
   13063 | zTXt  |      87 | Software..x...A.. ......B....} | 0xcd30861a
   13162 | IDAT  |    8192 | x...Y.$Wv&v.{.{l.T.......[w.=m | 0x1b2c97be
   21366 | IDAT  |    8192 | .4X.y.AR...4....:Ue..U.|1..:.. | 0xb3fecb3e
   29570 | IDAT  |    8192 | 'g.!... ...n...s..Jdz......... | 0xd040336d
   37774 | IDAT  |    8192 | ........k....CY/75I..u;.. .z.. | 0xf42a2c7d
   45978 | IDAT  |    8192 | .f>..]....UKqD2s.(.q....=x.l.\ | 0xc177fe83
   54182 | IDAT  |    8192 | .i.{!!B0...C!4.p..`D g`....... | 0x0e276268
   62386 | IDAT  |    8192 | .*.].4..Q..}(9...S0&.......T.9 | 0x297bb2db
   70590 | IDAT  |    8192 | ..k...6....g.1..}.].&.H....... | 0x05f6f4ef
   78794 | IDAT  |    8192 | .j..S.........z..!U.G0*.m%..09 | 0xe0946eb5
   86998 | IDAT  |    8192 | .....t.>!.....6^.<..;..?.$I..M | 0x843ecce0
   95202 | IDAT  |    8192 | W.&5.5J........FW`....3.N.9Pk; | 0x3a3dfeee
  103406 | IDAT  |    8192 | .....d.z".`...v=g-..-.c8...Z.5 | 0x65d6df49
  111610 | IDAT  |    8192 | .."...o<&."....1M....1&. ..5.. | 0x700b8cde
  119814 | IDAT  |    8192 | k........!..B*.....\*.(!..0.s. | 0x9b33b5b7
  128018 | IDAT  |    3346 | .Y.L@I$M.Z[.0A ...K#.t.0+.G(.j | 0x18044b20
  131376 | IEND  |       0 |                                | 0xae426082
""",
        """<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:dc="http://purl.org/dc/elements/1.1/">
   <dc:title>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">this is a title</rdf:li>
    </rdf:Alt>
   </dc:title>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>""",
        """STRUCTURE OF TIFF FILE (MM): $tif_bug_file
 address |    tag                              |      type |    count |    offset | value
      10 | 0x0100 ImageWidth                   |     SHORT |        1 |           | 40
      22 | 0x0101 ImageLength                  |     SHORT |        1 |           | 470
      34 | 0x0102 BitsPerSample                |     SHORT |        3 |       182 | 8 8 8
      46 | 0x0103 Compression                  |     SHORT |        1 |           | 5
      58 | 0x0106 PhotometricInterpretation    |     SHORT |        1 |           | 2
      70 | 0x0111 StripOffsets                 |      LONG |        1 |           | 2694
      82 | 0x0112 Orientation                  |     SHORT |        1 |           | 6
      94 | 0x0115 SamplesPerPixel              |     SHORT |        1 |           | 3
     106 | 0x0116 RowsPerStrip                 |     SHORT |        1 |           | 1092
     118 | 0x0117 StripByteCounts              |      LONG |        1 |           | 5086
     130 | 0x011c PlanarConfiguration          |     SHORT |        1 |           | 1
     142 | 0x013d Predictor                    |     SHORT |        1 |           | 2
     154 | 0x0153 SampleFormat                 |     SHORT |        3 |       188 | 1 1 1
     166 | 0x02bc XMLPacket                    |      BYTE |     2500 |       194 | <?xpacket begin="..." id="W5M0Mp ...
END $tif_bug_file
""",
        """<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?> <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2"> <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"> <rdf:Description rdf:about="" xmlns:xmp="http://ns.adobe.com/xap/1.0/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmp:Rating="0" xmp:ModifyDate="2015-02-13T20:46:51-06:00"> <dc:title> <rdf:Alt> <rdf:li xml:lang="x-default">the rest drink soda</rdf:li> <rdf:li xml:lang="en-GB">the Brits are in the bar</rdf:li> <rdf:li xml:lang="de-DE">the Germans like beer</rdf:li> </rdf:Alt> </dc:title> </rdf:Description> </rdf:RDF> </x:xmpmeta>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 <?xpacket end="w"?>""",
        """STRUCTURE OF JPEG FILE: $jpg_bug_file
 address | marker       |  length | data
       0 | 0xffd8 SOI  
       2 | 0xffe1 APP1  |   14862 | Exif..II*......................
   14866 | 0xffe1 APP1  |    2720 | http://ns.adobe.com/xap/1.0/.<?x
   17588 | 0xffed APP13 |     110 | Photoshop 3.0.8BIM.......6.....
   17700 | 0xffe2 APP2  |    4094 | MPF.II*...............0100.....
   21796 | 0xffdb DQT   |     132 
   21930 | 0xffc0 SOF0  |      17 
   21949 | 0xffc4 DHT   |     418 
   22369 | 0xffda SOS  
""",
    ]
    stderr = [""] * 8
    retval = [0] * 8

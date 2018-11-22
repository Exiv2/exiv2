# -*- coding: utf-8 -*-

import system_tests
import unittest

@unittest.skip("Skipping test using option -pR (only for Debug mode)")
class CheckDumpSubFiles(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1108"

    filenames = ["$data_path/exiv2-bug1108.exv",
                 "$data_path/imagemagick.png",
                 "$data_path/ReaganLargeTiff.tiff",
                 "$data_path/Reagan.jpg"
    ]

    commands = ["$exiv2 -pR " + filenames[0],
                "$exiv2 -pR " + filenames[1],
                "$exiv2 -pR " + filenames[2],
                "$exiv2 -pR " + filenames[3]
    ]

    stdout = [ """STRUCTURE OF JPEG FILE: """ + filenames[0] + """
 address | marker       |  length | data
       0 | 0xff01      
       7 | 0xffe1 APP1  |   15296 | Exif..II*......................
  STRUCTURE OF TIFF FILE (II): MemIo
   address |    tag                              |      type |    count |    offset | value
        10 | 0x010f Make                         |     ASCII |       18 |       146 | NIKON CORPORATION
        22 | 0x0110 Model                        |     ASCII |       12 |       164 | NIKON D5300
        34 | 0x0112 Orientation                  |     SHORT |        1 |           | 1
        46 | 0x011a XResolution                  |  RATIONAL |        1 |       176 | 300/1
        58 | 0x011b YResolution                  |  RATIONAL |        1 |       184 | 300/1
        70 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
        82 | 0x0131 Software                     |     ASCII |       10 |       192 | Ver.1.00 
        94 | 0x0132 DateTime                     |     ASCII |       20 |       202 | 2015:07:16 20:25:28
       106 | 0x0213 YCbCrPositioning             |     SHORT |        1 |           | 1
       118 | 0x8769 ExifTag                      |      LONG |        1 |           | 222
    STRUCTURE OF TIFF FILE (II): MemIo
     address |    tag                              |      type |    count |    offset | value
         224 | 0x829a ExposureTime                 |  RATIONAL |        1 |       732 | 10/4000
         236 | 0x829d FNumber                      |  RATIONAL |        1 |       740 | 100/10
         248 | 0x8822 ExposureProgram              |     SHORT |        1 |           | 0
         260 | 0x8827 ISOSpeedRatings              |     SHORT |        1 |           | 200
         272 | 0x8830 SensitivityType              |     SHORT |        1 |           | 2
         284 | 0x9000 ExifVersion                  | UNDEFINED |        4 |           | 0230
         296 | 0x9003 DateTimeOriginal             |     ASCII |       20 |       748 | 2015:07:16 15:38:54
         308 | 0x9004 DateTimeDigitized            |     ASCII |       20 |       768 | 2015:07:16 15:38:54
         320 | 0x9101 ComponentsConfiguration      | UNDEFINED |        4 |           | ...
         332 | 0x9102 CompressedBitsPerPixel       |  RATIONAL |        1 |       788 | 2/1
         344 | 0x9204 ExposureBiasValue            | SRATIONAL |        1 |       796 | 0/6
         356 | 0x9205 MaxApertureValue             |  RATIONAL |        1 |       804 | 43/10
         368 | 0x9207 MeteringMode                 |     SHORT |        1 |           | 5
         380 | 0x9208 LightSource                  |     SHORT |        1 |           | 0
         392 | 0x9209 Flash                        |     SHORT |        1 |           | 16
         404 | 0x920a FocalLength                  |  RATIONAL |        1 |       812 | 440/10
         416 | 0x927c MakerNote                    | UNDEFINED |     3152 |       820 | Nikon.....II*.....9.........0211 ...
      STRUCTURE OF TIFF FILE (II): MemIo
       address |    tag                              |      type |    count |    offset | value
            10 | 0x0001 Version                      | UNDEFINED |        4 |           | 0211
            22 | 0x0002 ISOSpeed                     |     SHORT |        2 |           | 0 200
            34 | 0x0004 Quality                      |     ASCII |        8 |       698 | NORMAL 
            46 | 0x0005 WhiteBalance                 |     ASCII |       13 |       706 | AUTO        
            58 | 0x0007 Focus                        |     ASCII |        7 |       720 | AF-A  
            70 | 0x0008 FlashSetting                 |     ASCII |       13 |       728 |             
            82 | 0x0009 GPSStatus                    |     ASCII |       20 |       742 |                    
            94 | 0x000b ProcessingSoftware           |    SSHORT |        2 |           | 0 0
           106 | 0x000c GPSSpeedRef                  |  RATIONAL |        4 |       762 | 538/256 354/256 256/256 256/256
           118 | 0x000d GPSSpeed                     | UNDEFINED |        4 |           | ...
           130 | 0x000e GPSTrackRef                  | UNDEFINED |        4 |           | ...
           142 | 0x0012 GPSMapDatum                  | UNDEFINED |        4 |           | ...
           154 | 0x0013 GPSDestLatitudeRef           |     SHORT |        2 |           | 0 200
           166 | 0x0016 GPSDestLongitude             |     SHORT |        4 |       794 | 0 0 6000 4000
           178 | 0x0017 GPSDestBearingRef            | UNDEFINED |        4 |           | ...
           190 | 0x0018 GPSDestBearing               | UNDEFINED |        4 |           | ...
           202 | 0x0019 GPSDestDistanceRef           | SRATIONAL |        1 |       802 | 0/6
           214 | 0x001b GPSProcessingMethod          |     SHORT |        7 |       810 | 0 6016 4016 6016 4016 ...
           226 | 0x001c GPSAreaInformation           | UNDEFINED |        3 |           | ...
           238 | 0x001d GPSDateStamp                 |     ASCII |        8 |       824 | 2567806
           250 | 0x001e GPSDifferential              |     SHORT |        1 |           | 1
           262 | 0x001f                              | UNDEFINED |        8 |       832 | 0100...
           274 | 0x0022                              |     SHORT |        1 |           | 65535
           286 | 0x0023                              | UNDEFINED |       58 |       840 | 0100STANDARD............STANDARD ...
           298 | 0x0024                              | UNDEFINED |        4 |           | ...
           310 | 0x0025                              | UNDEFINED |       14 |       898 | H.....H......
           322 | 0x002b                              | UNDEFINED |       16 |       912 | 0100...........
           334 | 0x002c                              | UNDEFINED |      574 |       928 | 0101#.......................... ...
           346 | 0x002d                              |     SHORT |        3 |      1502 | 512 0 0
           358 | 0x0032                              | UNDEFINED |        8 |      1508 | 0100...
           370 | 0x0035                              | UNDEFINED |        6 |      1516 | 0200.
           382 | 0x003b                              |  RATIONAL |        4 |      1522 | 256/256 256/256 256/256 256/256
           394 | 0x003c                              |     SHORT |        1 |           | 1
           406 | 0x0083                              |      BYTE |        1 |           | .
           418 | 0x0084                              |  RATIONAL |        4 |      1554 | 180/10 2500/10 35/10 63/10
           430 | 0x0087                              |      BYTE |        1 |           | 
           442 | 0x0089                              |     SHORT |        1 |           | 0
           454 | 0x008a                              |     SHORT |        1 |           | 1
           466 | 0x008b                              | UNDEFINED |        4 |           | 7..
           478 | 0x0095                              |     ASCII |        5 |      1586 | OFF 
           490 | 0x0097                              | UNDEFINED |     1188 |      1592 | 0219.dU....W..2......:.......F.# ...
           502 | 0x0098                              | UNDEFINED |       33 |      2780 | 0204.W....z.o..#[.....!o.x..E... ...
           514 | 0x009d                              |     SHORT |        1 |           | 0
           526 | 0x009e                              |     SHORT |       10 |      2814 | 0 0 0 0 0 ...
           538 | 0x00a2                              |      LONG |        1 |           | 6173648
           550 | 0x00a3                              |      BYTE |        1 |           | 
           562 | 0x00a7                              |      LONG |        1 |           | 9608
           574 | 0x00a8                              | UNDEFINED |       49 |      2834 | 0106........................... ...
           586 | 0x00ab                              |     ASCII |       16 |      2884 | AUTO(FLASH OFF)
           598 | 0x00b0                              | UNDEFINED |       16 |      2900 | 0100...........
           610 | 0x00b1                              |     SHORT |        1 |           | 4
           622 | 0x00b6                              | UNDEFINED |        8 |      2916 | .......
           634 | 0x00b7                              | UNDEFINED |       30 |      2924 | 0100....i....................
           646 | 0x00b8                              | UNDEFINED |      172 |      2954 | 0100..e........................ ...
           658 | 0x00bb                              | UNDEFINED |        8 |      3126 | 0200...
           670 | 0x00bf                              |     SHORT |        1 |           | 0
           682 | 0x00c0                              | UNDEFINED |        8 |      3134 | <......
      END MemIo
         428 | 0x9286 UserComment                  | UNDEFINED |       44 |      3972 | ASCII...                         ...
         440 | 0x9290 SubSecTime                   |     ASCII |        3 |           | 00
         452 | 0x9291 SubSecTimeOriginal           |     ASCII |        3 |           | 00
         464 | 0x9292 SubSecTimeDigitized          |     ASCII |        3 |           | 00
         476 | 0xa000 FlashpixVersion              | UNDEFINED |        4 |           | 0100
         488 | 0xa001 ColorSpace                   |     SHORT |        1 |           | 1
         500 | 0xa002 PixelXDimension              |     SHORT |        1 |           | 6000
         512 | 0xa003 PixelYDimension              |     SHORT |        1 |           | 4000
         524 | 0xa005 InteroperabilityTag          |      LONG |        1 |           | 4066
         536 | 0xa217 SensingMethod                |     SHORT |        1 |           | 2
         548 | 0xa300 FileSource                   | UNDEFINED |        1 |           | .
         560 | 0xa301 SceneType                    | UNDEFINED |        1 |           | .
         572 | 0xa302 CFAPattern                   | UNDEFINED |        8 |      4016 | ........
         584 | 0xa401 CustomRendered               |     SHORT |        1 |           | 0
         596 | 0xa402 ExposureMode                 |     SHORT |        1 |           | 0
         608 | 0xa403 WhiteBalance                 |     SHORT |        1 |           | 0
         620 | 0xa404 DigitalZoomRatio             |  RATIONAL |        1 |      4024 | 1/1
         632 | 0xa405 FocalLengthIn35mmFilm        |     SHORT |        1 |           | 66
         644 | 0xa406 SceneCaptureType             |     SHORT |        1 |           | 0
         656 | 0xa407 GainControl                  |     SHORT |        1 |           | 0
         668 | 0xa408 Contrast                     |     SHORT |        1 |           | 0
         680 | 0xa409 Saturation                   |     SHORT |        1 |           | 0
         692 | 0xa40a Sharpness                    |     SHORT |        1 |           | 0
         704 | 0xa40c SubjectDistanceRange         |     SHORT |        1 |           | 0
         716 | 0xa420 ImageUniqueID                |     ASCII |       33 |      4032 | 090caaf2c085f3e102513b24750041aa ...
    END MemIo
       130 | 0x8825 GPSTag                       |      LONG |        1 |           | 4096
      4346 | 0x0103 Compression                  |     SHORT |        1 |           | 6
      4358 | 0x011a XResolution                  |  RATIONAL |        1 |      4434 | 300/1
      4370 | 0x011b YResolution                  |  RATIONAL |        1 |      4442 | 300/1
      4382 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
      4394 | 0x0201 JPEGInterchangeFormat        |      LONG |        1 |           | 4450
      4406 | 0x0202 JPEGInterchangeFormatLength  |      LONG |        1 |           | 10837
      4418 | 0x0213 YCbCrPositioning             |     SHORT |        1 |           | 1
  END MemIo
   15305 | 0xffe1 APP1  |    2610 | http://ns.adobe.com/xap/1.0/.<?x
   17917 | 0xffed APP13 |      68 | Photoshop 3.0.8BIM.......'.....
  Record | DataSet | Name                     | Length | Data
       1 |       0 | ModelVersion             |      2 | ..
       1 |      90 | CharacterSet             |      3 | .%G
       2 |       0 | RecordVersion            |      2 | ..
       2 |     120 | Caption                  |     12 | Classic View
   17987 | 0xffd9 EOI  
""",
    """STRUCTURE OF PNG FILE: """ + filenames[1] + """
 address | chunk |  length | data                           | checksum
       8 | IHDR  |      13 | ...@........                   | 0x7f775da4
      33 | iCCP  |    1404 | icc..x...i8........af\...w_3.. | 0x363e2409
    1449 | sBIT  |       3 | ...                            | 0xdbe14fe0
    1464 | zTXt  |      87 | Software..x...A.. ......B....} | 0xcd30861a
Software: digiKam 0.9.0-svn ( libpng version 1.2.8 - December 3, 2004 (header) )
    1563 | tEXt  |   24482 | Raw profile type exif..exif.   | 0x81743ec0
  STRUCTURE OF TIFF FILE (MM): MemIo
   address |    tag                              |      type |    count |    offset | value
        10 | 0x0100 ImageWidth                   |     SLONG |        1 |           | 320
        22 | 0x0101 ImageLength                  |     SLONG |        1 |           | 211
        34 | 0x010f Make                         |     ASCII |       18 |       146 | NIKON CORPORATION
        46 | 0x0110 Model                        |     ASCII |       10 |       164 | NIKON D70
        58 | 0x0112 Orientation                  |     SHORT |        1 |           | 1
        70 | 0x011a XResolution                  |  RATIONAL |        1 |       174 | 300/1
        82 | 0x011b YResolution                  |  RATIONAL |        1 |       182 | 300/1
        94 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
       106 | 0x0131 Software                     |     ASCII |       18 |       190 | digiKam-0.9.0-svn
       118 | 0x0132 DateTime                     |     ASCII |       20 |       208 | 2006:02:04 16:09:30
       130 | 0x8769 ExifTag                      |      LONG |        1 |           | 228
    STRUCTURE OF TIFF FILE (MM): MemIo
     address |    tag                              |      type |    count |    offset | value
         230 | 0x829a ExposureTime                 |  RATIONAL |        1 |       546 | 1/4
         242 | 0x829d FNumber                      |  RATIONAL |        1 |       554 | 22/1
         254 | 0x8822 ExposureProgram              |     SHORT |        1 |           | 4
         266 | 0x8827 ISOSpeedRatings              |     SHORT |        1 |           | 200
         278 | 0x9003 DateTimeOriginal             |     ASCII |       20 |       562 | 2006:02:04 16:09:30
         290 | 0x9004 DateTimeDigitized            |     ASCII |       20 |       582 | 2006:02:04 16:09:30
         302 | 0x9201 ShutterSpeedValue            |  RATIONAL |        1 |       602 | 2/1
         314 | 0x9202 ApertureValue                |  RATIONAL |        1 |       610 | 4676053/524288
         326 | 0x9204 ExposureBiasValue            |  RATIONAL |        1 |       618 | 11184811/33554432
         338 | 0x9205 MaxApertureValue             |  RATIONAL |        1 |       626 | 12460663/4194304
         350 | 0x9208 LightSource                  |     SHORT |        1 |           | 0
         362 | 0x9209 Flash                        |     SHORT |        1 |           | 0
         374 | 0x920a FocalLength                  |  RATIONAL |        1 |       634 | 50/1
         386 | 0x927c MakerNote                    | UNDEFINED |     6989 |       642 | Nikon.....MM.*.....+........0210 ...
      STRUCTURE OF TIFF FILE (MM): MemIo
       address |    tag                              |      type |    count |    offset | value
            10 | 0x0001 Version                      | UNDEFINED |        4 |           | 0210
            22 | 0x0002 ISOSpeed                     |     SHORT |        2 |           | 0 200
            34 | 0x0004 Quality                      |     ASCII |        8 |       530 | RAW    
            46 | 0x0005 WhiteBalance                 |     ASCII |       13 |       538 | AUTO        
            58 | 0x0006 Sharpening                   |     ASCII |        7 |       551 | NORMAL
            70 | 0x0007 Focus                        |     ASCII |        7 |       558 | AF-S  
            82 | 0x0008 FlashSetting                 |     ASCII |       13 |       565 | NORMAL      
            94 | 0x0009 GPSStatus                    |     ASCII |       13 |       578 |             
           106 | 0x000b ProcessingSoftware           |    SSHORT |        1 |           | 0
           118 | 0x000d GPSSpeed                     | UNDEFINED |        4 |           | ...
           130 | 0x000e GPSTrackRef                  | UNDEFINED |        4 |           | ...
           142 | 0x0011 GPSImgDirection              |      LONG |        1 |           | 7006
           154 | 0x0012 GPSMapDatum                  | UNDEFINED |        4 |           | ...
           166 | 0x0013 GPSDestLatitudeRef           |     SHORT |        2 |           | 0 200
           178 | 0x0017 GPSDestBearingRef            | UNDEFINED |        4 |           | ...
           190 | 0x0018 GPSDestBearing               | UNDEFINED |        4 |           | ...
           202 | 0x0019 GPSDestDistanceRef           | SRATIONAL |        1 |       591 | 0/1
           214 | 0x0081                              |     ASCII |        9 |       599 | NORMAL  
           226 | 0x0083                              |      BYTE |        1 |           | .
           238 | 0x0084                              |  RATIONAL |        4 |       608 | 180/10 500/10 28/10 28/10
           250 | 0x0087                              |      BYTE |        1 |           | 
           262 | 0x0088 AFFocusPos                   | UNDEFINED |        4 |           | ....
           274 | 0x0089                              |     SHORT |        1 |           | 0
           286 | 0x008b                              | UNDEFINED |        4 |           | H..
           298 | 0x008c                              | UNDEFINED |     4160 |       640 | I.............................. ...
           310 | 0x008d                              |     ASCII |        9 |      4800 | MODE2   
           322 | 0x0090                              |     ASCII |       12 |      4809 | NATURAL    
           334 | 0x0091                              | UNDEFINED |      465 |      4821 | 0103........................... ...
           346 | 0x0092                              |    SSHORT |        1 |           | 0
           358 | 0x0093                              |     SHORT |        1 |           | 1
           370 | 0x0095                              |     ASCII |        5 |      5286 | OFF 
           382 | 0x0096                              | UNDEFINED |     1412 |      5291 | D..H.H.H.H...................... ...
           394 | 0x0097                              | UNDEFINED |      140 |      6703 | 0103.................:...z...... ...
           406 | 0x0098                              | UNDEFINED |       31 |      6843 | ...............................
           418 | 0x0099                              |     SHORT |        2 |           | 1520 1008
           430 | 0x009a                              |  RATIONAL |        2 |      6874 | 19398659/1 131072/19988482
           442 | 0x00a0                              |     ASCII |       21 |      6890 | .........2...........
           454 | 0x00a4                              | UNDEFINED |        4 |           | 0200
           466 | 0x00a7                              |      LONG |        1 |           | 5670
           478 | 0x00a8                              | UNDEFINED |       20 |      6911 | ..........a~NIKON CO
           490 | 0x00a9                              |     ASCII |       16 |      6931 | RPORATION.NIKON 
           502 | 0x00aa                              |     ASCII |       16 |      6947 | D70....,.......,
           514 | 0x00ab                              |     ASCII |       16 |      6963 | ....Bibble 4.5.
      END MemIo
         398 | 0xa002 PixelXDimension              |     SLONG |        1 |           | 320
         410 | 0xa003 PixelYDimension              |     SLONG |        1 |           | 211
         422 | 0xa217 SensingMethod                |      BYTE |        1 |           | .
         434 | 0xa301 SceneType                    |      BYTE |        1 |           | .
         446 | 0xa402 ExposureMode                 |     SHORT |        1 |           | 0
         458 | 0xa403 WhiteBalance                 |     SHORT |        1 |           | 0
         470 | 0xa405 FocalLengthIn35mmFilm        |     SHORT |        1 |           | 75
         482 | 0xa406 SceneCaptureType             |     SHORT |        1 |           | 0
         494 | 0xa408 Contrast                     |     SHORT |        1 |           | 0
         506 | 0xa409 Saturation                   |     SHORT |        1 |           | 0
         518 | 0xa40a Sharpness                    |     SHORT |        1 |           | 0
         530 | 0xa40c SubjectDistanceRange         |     SHORT |        1 |           | 0
    END MemIo
      7633 | 0x0103 Compression                  |     SHORT |        1 |           | 6
      7645 | 0x0201 JPEGInterchangeFormat        |      LONG |        1 |           | 7673
      7657 | 0x0202 JPEGInterchangeFormatLength  |      LONG |        1 |           | 4376
  END MemIo
   26057 | tEXt  |     471 | Raw profile type iptc..iptc.   | 0x2db5653b
  Record | DataSet | Name                     | Length | Data
       2 |       5 | ObjectName               |      4 | ovni
       2 |      10 | Urgency                  |      1 | 0
       2 |      15 | Category                 |      3 | I -
       2 |      20 | SuppCategory             |     13 | SPCL Specials
       2 |     116 | Copyright                |      4 | E.T.
       2 |     110 | Credit                   |      9 | Spielberg
       2 |     122 | Writer                   |     21 | L'ovni de la Bastille
       2 |      90 | City                     |      5 | Paris
       2 |      95 | ProvinceState            |     13 | ..le de Franc
       2 |     101 | CountryName              |     12 | FRA - France
       2 |     100 | CountryCode              |      3 | 750
       2 |      40 | SpecialInstructions      |     10 | Y'en a pas
       2 |     103 | TransmissionReference    |      2 | 36
       2 |     115 | Source                   |      8 | Chez moi
       2 |      55 | DateCreated              |      8 | 20060204
       2 |      60 | TimeCreated              |     11 | 160930+0000
       2 |       0 | RecordVersion            |      2 | ..
   26540 | IDAT  |    8192 | x...Y.$Wv&v.{.{l.T.......[w.=m | 0x1b2c97be
   34744 | IDAT  |    8192 | .4X.y.AR...4....:Ue..U.|1..:.. | 0xb3fecb3e
   42948 | IDAT  |    8192 | 'g.!... ...n...s..Jdz......... | 0xd040336d
   51152 | IDAT  |    8192 | ........k....CY/75I..u;.. .z.. | 0xf42a2c7d
   59356 | IDAT  |    8192 | .f>..]....UKqD2s.(.q....=x.l.\ | 0xc177fe83
   67560 | IDAT  |    8192 | .i.{!!B0...C!4.p..`D g`....... | 0x0e276268
   75764 | IDAT  |    8192 | .*.].4..Q..}(9...S0&.......T.9 | 0x297bb2db
   83968 | IDAT  |    8192 | ..k...6....g.1..}.].&.H....... | 0x05f6f4ef
   92172 | IDAT  |    8192 | .j..S.........z..!U.G0*.m%..09 | 0xe0946eb5
  100376 | IDAT  |    8192 | .....t.>!.....6^.<..;..?.$I..M | 0x843ecce0
  108580 | IDAT  |    8192 | W.&5.5J........FW`....3.N.9Pk; | 0x3a3dfeee
  116784 | IDAT  |    8192 | .....d.z".`...v=g-..-.c8...Z.5 | 0x65d6df49
  124988 | IDAT  |    8192 | .."...o<&."....1M....1&. ..5.. | 0x700b8cde
  133192 | IDAT  |    8192 | k........!..B*.....\*.(!..0.s. | 0x9b33b5b7
  141396 | IDAT  |    3346 | .Y.L@I$M.Z[.0A ...K#.t.0+.G(.j | 0x18044b20
  144754 | IEND  |       0 |                                | 0xae426082
""",
    """STRUCTURE OF TIFF FILE (II): """ + filenames[2] + """
 address |    tag                              |      type |    count |    offset | value
      10 | 0x00fe NewSubfileType               |      LONG |        1 |           | 0
      22 | 0x0100 ImageWidth                   |     SHORT |        1 |           | 200
      34 | 0x0101 ImageLength                  |     SHORT |        1 |           | 130
      46 | 0x0102 BitsPerSample                |     SHORT |        3 |       374 | 8 8 8
      58 | 0x0103 Compression                  |     SHORT |        1 |           | 5
      70 | 0x0106 PhotometricInterpretation    |     SHORT |        1 |           | 2
      82 | 0x010d DocumentName                 |     ASCII |       59 |       380 | /media/trekstor/exiv2/exiv2-svn/ ...
      94 | 0x010e ImageDescription             |     ASCII |      403 |       440 | 040621-N-6536T-062.USS Ronald Re ...
     106 | 0x010f Make                         |     ASCII |       18 |       844 | NIKON CORPORATION
     118 | 0x0110 Model                        |     ASCII |       10 |       862 | NIKON D1X
     130 | 0x0111 StripOffsets                 |      LONG |        2 |       872 | 1623650 1667534
     142 | 0x0112 Orientation                  |     SHORT |        1 |           | 1
     154 | 0x0115 SamplesPerPixel              |     SHORT |        1 |           | 3
     166 | 0x0116 RowsPerStrip                 |     SHORT |        1 |           | 128
     178 | 0x0117 StripByteCounts              |      LONG |        2 |       880 | 43883 931
     190 | 0x011a XResolution                  |  RATIONAL |        1 |       888 | 300/1
     202 | 0x011b YResolution                  |  RATIONAL |        1 |       896 | 300/1
     214 | 0x011c PlanarConfiguration          |     SHORT |        1 |           | 1
     226 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
     238 | 0x0131 Software                     |     ASCII |       11 |       904 | GIMP 2.9.5
     250 | 0x0132 DateTime                     |     ASCII |       20 |       916 | 2016:09:13 12:23:27
     262 | 0x013b Artist                       |     ASCII |       34 |       936 | Photographer..s Mate 3rd Class ( ...
     274 | 0x013d Predictor                    |     SHORT |        1 |           | 2
     286 | 0x014a SubIFDs                      |      LONG |        1 |           | 1622480
  STRUCTURE OF TIFF FILE (II): """ + filenames[2] + """
   address |    tag                              |      type |    count |    offset | value
   1622482 | 0x00fe NewSubfileType               |      LONG |        1 |           | 1
   1622494 | 0x0100 ImageWidth                   |     SHORT |        1 |           | 256
   1622506 | 0x0101 ImageLength                  |     SHORT |        1 |           | 166
   1622518 | 0x0102 BitsPerSample                |     SHORT |        3 |   1622630 | 8 8 8
   1622530 | 0x0103 Compression                  |     SHORT |        1 |           | 1
   1622542 | 0x0106 PhotometricInterpretation    |     SHORT |        1 |           | 2
   1622554 | 0x0111 StripOffsets                 |      LONG |        1 |           | 1668466
   1622566 | 0x0115 SamplesPerPixel              |     SHORT |        1 |           | 3
   1622578 | 0x0116 RowsPerStrip                 |     SHORT |        1 |           | 166
   1622590 | 0x0117 StripByteCounts              |      LONG |        1 |           | 127488
   1622602 | 0x011c PlanarConfiguration          |     SHORT |        1 |           | 1
   1622614 | 0x0153 SampleFormat                 |     SHORT |        3 |   1622636 | 1 1 1
  END """ + filenames[2] + """
     298 | 0x0153 SampleFormat                 |     SHORT |        3 |       970 | 1 1 1
     310 | 0x02bc XMLPacket                    |      BYTE |     7135 |       976 | <?xpacket begin="..." id="W5M0Mp ...
     322 | 0x83bb IPTCNAA                      |      LONG |      192 |      8112 | 5898524 1193614083 5243420 1869107232 1919381364 ...
  Record | DataSet | Name                     | Length | Data
       1 |      90 | CharacterSet             |      3 | .%G
       2 |      80 | Byline                   |     32 | Photographer..s Mate 3rd Class (
       2 |      85 | BylineTitle              |     21 | U.S Navy Photographer
       2 |     120 | Caption                  |    402 | 040621-N-6536T-062.USS Ronald Reagan (CV...
     334 | 0x8769 ExifTag                      |      LONG |        1 |           | 1622642
  STRUCTURE OF TIFF FILE (II): """ + filenames[2] + """
   address |    tag                              |      type |    count |    offset | value
   1622644 | 0x829a ExposureTime                 |  RATIONAL |        1 |   1623092 | 1/125
   1622656 | 0x829d FNumber                      |  RATIONAL |        1 |   1623100 | 5/1
   1622668 | 0x8822 ExposureProgram              |     SHORT |        1 |           | 1
   1622680 | 0x9000 ExifVersion                  | UNDEFINED |        4 |           | 0220
   1622692 | 0x9003 DateTimeOriginal             |     ASCII |       20 |   1623108 | 2004:06:21 23:37:53
   1622704 | 0x9004 DateTimeDigitized            |     ASCII |       20 |   1623128 | 2004:06:21 23:37:53
   1622716 | 0x9101 ComponentsConfiguration      | UNDEFINED |        4 |           | ...
   1622728 | 0x9102 CompressedBitsPerPixel       |  RATIONAL |        1 |   1623148 | 4/1
   1622740 | 0x9204 ExposureBiasValue            | SRATIONAL |        1 |   1623156 | 1/3
   1622752 | 0x9205 MaxApertureValue             |  RATIONAL |        1 |   1623164 | 3/1
   1622764 | 0x9207 MeteringMode                 |     SHORT |        1 |           | 2
   1622776 | 0x9208 LightSource                  |     SHORT |        1 |           | 10
   1622788 | 0x9209 Flash                        |     SHORT |        1 |           | 0
   1622800 | 0x920a FocalLength                  |  RATIONAL |        1 |   1623172 | 42/1
   1622812 | 0x9286 UserComment                  | UNDEFINED |      410 |   1623180 | ........040621-N-6536T-062.USS R ...
   1622824 | 0x9290 SubSecTime                   |     ASCII |        3 |           | 06
   1622836 | 0x9291 SubSecTimeOriginal           |     ASCII |        3 |           | 06
   1622848 | 0x9292 SubSecTimeDigitized          |     ASCII |        3 |           | 06
   1622860 | 0xa000 FlashpixVersion              | UNDEFINED |        4 |           | 0100
   1622872 | 0xa001 ColorSpace                   |     SHORT |        1 |           | 1
   1622884 | 0xa002 PixelXDimension              |      LONG |        1 |           | 200
   1622896 | 0xa003 PixelYDimension              |      LONG |        1 |           | 130
   1622908 | 0xa217 SensingMethod                |     SHORT |        1 |           | 2
   1622920 | 0xa300 FileSource                   | UNDEFINED |        1 |           | .
   1622932 | 0xa301 SceneType                    | UNDEFINED |        1 |           | .
   1622944 | 0xa401 CustomRendered               |     SHORT |        1 |           | 0
   1622956 | 0xa402 ExposureMode                 |     SHORT |        1 |           | 1
   1622968 | 0xa403 WhiteBalance                 |     SHORT |        1 |           | 1
   1622980 | 0xa404 DigitalZoomRatio             |  RATIONAL |        1 |   1623590 | 1/1
   1622992 | 0xa405 FocalLengthIn35mmFilm        |     SHORT |        1 |           | 63
   1623004 | 0xa406 SceneCaptureType             |     SHORT |        1 |           | 0
   1623016 | 0xa407 GainControl                  |     SHORT |        1 |           | 0
   1623028 | 0xa408 Contrast                     |     SHORT |        1 |           | 0
   1623040 | 0xa409 Saturation                   |     SHORT |        1 |           | 0
   1623052 | 0xa40a Sharpness                    |     SHORT |        1 |           | 0
   1623064 | 0xa40c SubjectDistanceRange         |     SHORT |        1 |           | 0
   1623076 | 0xa420 ImageUniqueID                |     ASCII |       33 |   1623598 | 127c1377b054a3f65bf2754ebb24e7f2 ...
  END """ + filenames[2] + """
     346 | 0x8773 InterColorProfile            | UNDEFINED |  1613600 |      8880 | ... APPL....prtrRGB Lab ........ ...
     358 | 0x8825 GPSTag                       |      LONG |        1 |           | 1623632
END """ + filenames[2] + """
""",
    """STRUCTURE OF JPEG FILE: """ + filenames[3] + """
 address | marker       |  length | data
       0 | 0xffd8 SOI  
       2 | 0xffe1 APP1  |    5718 | Exif..MM.*......................
  STRUCTURE OF TIFF FILE (MM): MemIo
   address |    tag                              |      type |    count |    offset | value
        10 | 0x0100 ImageWidth                   |     SHORT |        1 |           | 200
        22 | 0x0101 ImageLength                  |     SHORT |        1 |           | 130
        34 | 0x0102 BitsPerSample                |     SHORT |        4 |       242 | 8 8 8 8
        46 | 0x0103 Compression                  |     SHORT |        1 |           | 1
        58 | 0x0106 PhotometricInterpretation    |     SHORT |        1 |           | 2
        70 | 0x010e ImageDescription             |     ASCII |      403 |       250 | 040621-N-6536T-062.USS Ronald Re ...
        82 | 0x010f Make                         |     ASCII |       18 |       653 | NIKON CORPORATION
        94 | 0x0110 Model                        |     ASCII |       10 |       671 | NIKON D1X
       106 | 0x0112 Orientation                  |     SHORT |        1 |           | 1
       118 | 0x0115 SamplesPerPixel              |     SHORT |        1 |           | 4
       130 | 0x011a XResolution                  |  RATIONAL |        1 |       681 | 3000000/10000
       142 | 0x011b YResolution                  |  RATIONAL |        1 |       689 | 3000000/10000
       154 | 0x011c PlanarConfiguration          |     SHORT |        1 |           | 1
       166 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
       178 | 0x0131 Software                     |     ASCII |       40 |       697 | Adobe Photoshop Elements 12.0 Ma ...
       190 | 0x0132 DateTime                     |     ASCII |       20 |       737 | 2016:09:13 11:58:16
       202 | 0x013b Artist                       |     ASCII |       34 |       757 | Photographer..s Mate 3rd Class ( ...
       214 | 0x8769 ExifTag                      |      LONG |        1 |           | 792
    STRUCTURE OF TIFF FILE (MM): MemIo
     address |    tag                              |      type |    count |    offset | value
         794 | 0x829a ExposureTime                 |  RATIONAL |        1 |      1254 | 1/125
         806 | 0x829d FNumber                      |  RATIONAL |        1 |      1262 | 5/1
         818 | 0x8822 ExposureProgram              |     SHORT |        1 |           | 1
         830 | 0x9000 ExifVersion                  | UNDEFINED |        4 |           | 0220
         842 | 0x9003 DateTimeOriginal             |     ASCII |       20 |      1270 | 2004:06:21 23:37:53
         854 | 0x9004 DateTimeDigitized            |     ASCII |       20 |      1290 | 2004:06:21 23:37:53
         866 | 0x9101 ComponentsConfiguration      | UNDEFINED |        4 |           | ...
         878 | 0x9102 CompressedBitsPerPixel       |  RATIONAL |        1 |      1310 | 4/1
         890 | 0x9201 ShutterSpeedValue            | SRATIONAL |        1 |      1318 | 6965784/1000000
         902 | 0x9202 ApertureValue                |  RATIONAL |        1 |      1326 | 4643856/1000000
         914 | 0x9204 ExposureBiasValue            | SRATIONAL |        1 |      1334 | 1/3
         926 | 0x9205 MaxApertureValue             |  RATIONAL |        1 |      1342 | 3/1
         938 | 0x9207 MeteringMode                 |     SHORT |        1 |           | 2
         950 | 0x9208 LightSource                  |     SHORT |        1 |           | 10
         962 | 0x9209 Flash                        |     SHORT |        1 |           | 0
         974 | 0x920a FocalLength                  |  RATIONAL |        1 |      1350 | 42/1
         986 | 0x9290 SubSecTime                   |     ASCII |        3 |           | 06
         998 | 0x9291 SubSecTimeOriginal           |     ASCII |        3 |           | 06
        1010 | 0x9292 SubSecTimeDigitized          |     ASCII |        3 |           | 06
        1022 | 0xa000 FlashpixVersion              | UNDEFINED |        4 |           | 0100
        1034 | 0xa001 ColorSpace                   |     SHORT |        1 |           | 65535
        1046 | 0xa002 PixelXDimension              |      LONG |        1 |           | 200
        1058 | 0xa003 PixelYDimension              |      LONG |        1 |           | 130
        1070 | 0xa217 SensingMethod                |     SHORT |        1 |           | 2
        1082 | 0xa300 FileSource                   | UNDEFINED |        1 |           | .
        1094 | 0xa301 SceneType                    | UNDEFINED |        1 |           | .
        1106 | 0xa401 CustomRendered               |     SHORT |        1 |           | 0
        1118 | 0xa402 ExposureMode                 |     SHORT |        1 |           | 1
        1130 | 0xa403 WhiteBalance                 |     SHORT |        1 |           | 1
        1142 | 0xa404 DigitalZoomRatio             |  RATIONAL |        1 |      1358 | 1/1
        1154 | 0xa405 FocalLengthIn35mmFilm        |     SHORT |        1 |           | 63
        1166 | 0xa406 SceneCaptureType             |     SHORT |        1 |           | 0
        1178 | 0xa407 GainControl                  |     SHORT |        1 |           | 0
        1190 | 0xa408 Contrast                     |     SHORT |        1 |           | 0
        1202 | 0xa409 Saturation                   |     SHORT |        1 |           | 0
        1214 | 0xa40a Sharpness                    |     SHORT |        1 |           | 0
        1226 | 0xa40c SubjectDistanceRange         |     SHORT |        1 |           | 0
        1238 | 0xa420 ImageUniqueID                |     ASCII |       33 |      1366 | 127c1377b054a3f65bf2754ebb24e7f2 ...
    END MemIo
       226 | 0x8825 GPSTag                       |      LONG |        1 |           | 1400
      1422 | 0x0103 Compression                  |     SHORT |        1 |           | 6
      1434 | 0x011a XResolution                  |  RATIONAL |        1 |      1498 | 72/1
      1446 | 0x011b YResolution                  |  RATIONAL |        1 |      1506 | 72/1
      1458 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
      1470 | 0x0201 JPEGInterchangeFormat        |      LONG |        1 |           | 1514
      1482 | 0x0202 JPEGInterchangeFormatLength  |      LONG |        1 |           | 4196
  END MemIo
    5722 | 0xffed APP13 |    3038 | Photoshop 3.0.8BIM..........Z...
  Record | DataSet | Name                     | Length | Data
       1 |      90 | CharacterSet             |      3 | .%G
       1 |      90 | CharacterSet             |      3 | .%G
       2 |       0 | RecordVersion            |      2 | ..
       2 |     120 | Caption                  |    402 | 040621-N-6536T-062.USS Ronald Reagan (CV...
       2 |     122 | Writer                   |      9 | Dir. NVNS
       2 |      40 | SpecialInstructions      |     49 | Credit as U.S. Navy photo by Elizabeth T...
       2 |      80 | Byline                   |     32 | Photographer..s Mate 3rd Class (
       2 |      85 | BylineTitle              |     21 | U.S Navy Photographer
       2 |     110 | Credit                   |      8 | U.S Navy
       2 |     115 | Source                   |     24 | Navy Visual News Service
       2 |       5 | ObjectName               |     18 | 040621-N-6536T-062
       2 |      55 | DateCreated              |      8 | 20040621
       2 |      60 | TimeCreated              |     11 | 000000+0000
       2 |      62 | DigitizationDate         |      8 | 20040621
       2 |      63 | DigitizationTime         |     11 | 233753-0400
       2 |      90 | City                     |     19 | Straits of Magellan
       2 |     101 | CountryName              |     13 | South America
       2 |      15 | Category                 |      1 | N
       2 |      20 | SuppCategory             |     12 | 703-614-9154
       2 |      20 | SuppCategory             |     23 | navyvisualnews@navy.mil
       2 |      20 | SuppCategory             |     11 | UNCLASSFIED
       2 |      10 | Urgency                  |      1 | 5
       2 |      25 | Keywords                 |     13 | ronald reagan
       2 |      25 | Keywords                 |      6 | reagan
       2 |      25 | Keywords                 |      6 | cvn 76
       2 |      25 | Keywords                 |      6 | cvn-76
       2 |      25 | Keywords                 |     18 | straights magellan
       2 |      25 | Keywords                 |      8 | magellan
       2 |      25 | Keywords                 |      7 | carrier
       2 |      25 | Keywords                 |     12 | nimitz-class
       2 |      25 | Keywords                 |      4 | ship
       2 |      25 | Keywords                 |      8 | underway
    8762 | 0xffe1 APP1  |    5329 | http://ns.adobe.com/xap/1.0/.<?x
   14093 | 0xffe2 APP2  |     576 | ICC_PROFILE......0ADBE....mntrRG chunk 1/1
   14671 | 0xffee APP14 |      14 | Adobe.d@......
   14687 | 0xffdb DQT   |     132 
   14821 | 0xffc0 SOF0  |      17 
   14840 | 0xffdd DRI   |       4 
   14846 | 0xffc4 DHT   |     418 
   15266 | 0xffda SOS  
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

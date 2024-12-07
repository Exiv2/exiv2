# -*- coding: utf-8 -*-

import system_tests
import unittest

# test needs system_tests.BT.vv['enable_bmff']=1
bSkip = system_tests.BT.verbose_version().get("enable_bmff") != "1"
if bSkip:
    raise unittest.SkipTest("*** requires enable_bmff=1 ***")


class pull_2841_hej2_bmff(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/2841"
    filename = "$data_path/Reagan.hej2"
    commands = ["$exiv2  -pS       $filename", "$exiv2            $filename"]
    retval = [0] * len(commands)
    stderr = [""] * len(commands)
    stdin = [""] * len(commands)
    stdout = [
        """Exiv2::BmffImage::boxHandler: ftyp        0->28 brand: j2ki
Exiv2::BmffImage::boxHandler: meta       28->397 
  Exiv2::BmffImage::boxHandler: hdlr       40->33 
  Exiv2::BmffImage::boxHandler: pitm       73->14 
  Exiv2::BmffImage::boxHandler: iloc       87->70 
        95 |       18 |   ID |    1 |    433,  2833
       113 |       18 |   ID |    2 |   3266,  5714
       131 |       18 |   ID |    3 |   8980,  5298
  Exiv2::BmffImage::boxHandler: iinf      157->98 
    Exiv2::BmffImage::boxHandler: infe      171->21 ID =   1 j2k1 
    Exiv2::BmffImage::boxHandler: infe      192->21 ID =   2 Exif  *** Exif ***
    Exiv2::BmffImage::boxHandler: infe      213->42 ID =   3 mime  *** XMP ***
  Exiv2::BmffImage::boxHandler: iprp      255->130 
    Exiv2::BmffImage::boxHandler: ipco      263->99 
      Exiv2::BmffImage::boxHandler: ispe      271->20 pixelWidth_, pixelHeight_ = 200, 130
      Exiv2::BmffImage::boxHandler: colr      291->19 
      Exiv2::BmffImage::boxHandler: j2kH      310->36 
      Exiv2::BmffImage::boxHandler: pixi      346->16 
    Exiv2::BmffImage::boxHandler: ipma      362->23 
  Exiv2::BmffImage::boxHandler: iref      385->40 
Exiv2::BMFF Exif: ID = 2 from,length = 3266,5714
Exiv2::BMFF XMP: ID = 3 from,length = 8980,5298
Exiv2::BmffImage::boxHandler: mdat      425->13853 
""",
        """File name       : $filename
File size       : 14278 Bytes
MIME type       : image/hej2k
Image size      : 200 x 130
Thumbnail       : image/jpeg, 4196 Bytes
Camera make     : NIKON CORPORATION
Camera model    : NIKON D1X
Image timestamp : 2004:06:21 23:37:53
File number     : 
Exposure time   : 1/125 s
Aperture        : F5
Exposure bias   : +1/3 EV
Flash           : No flash
Flash bias      : 
Focal length    : 42.0 mm
Subject distance: 
ISO speed       : 
Exposure mode   : Manual
Metering mode   : Center weighted average
Macro mode      : 
Image quality   : 
White balance   : Manual
Copyright       : 
Exif comment    : 

""",
    ]

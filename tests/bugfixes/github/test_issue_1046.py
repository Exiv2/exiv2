# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, path
@CopyFiles("$data_path/IMG_0246.exv")

class test_issue_1046Test(metaclass=CaseMeta):

    filename = path("$data_path/IMG_0246_copy.exv")
    # Use the """ string syntax for -M"set Bla.Dee.Bla" commands to ensure they run on Windows!
    commands  = [  "$exiv2 -pt -b -g GPSInfo -g UserCom    $filename"
                ,  "$exiv2 -pv -b -g GPSInfo -g UserCom    $filename"
                ,"""$exiv2 -M"set Exif.Photo.UserComment charset=Ascii how now brown cow" $filename"""
                ,  "$exiv2 -pt -b           -g UserComment $filename"
                ,  "$exiv2 -pv -b           -g UserComment $filename"
                ,"""$exiv2 -M"set Exif.Photo.UserComment charset=Unicode \u0041\u0042" $filename"""
                ,  "$exiv2 -pt -b           -g UserComment $filename"
                ,  "$exiv2 -pv -b           -g UserComment $filename"
                ,"""$exiv2 -M"set Exif.Photo.UserComment hello world" $filename"""
                ,  "$exiv2 -pt -b           -g UserComment $filename"
                ,  "$exiv2 -pv -b           -g UserComment $filename"
                ,"""$exiv2 -M"set Exif.GPSInfo.GPSProcessingMethod charset=Unicode \u0052\u006f\u0062\u0069\u006e" $filename"""
                ,"""$exiv2 -M"set Exif.GPSInfo.GPSAreaInformation  charset=Ascii area information"                 $filename"""
                ,  "$exiv2 -pt -b -g GPSPro -g GPSArea      $filename"
                ,  "$exiv2 -pv -b -g GPSPro -g GPSArea      $filename"
                ]
    stdout = ["""Exif.Photo.UserComment                       Undefined  12  charset=Unicode AB
Exif.GPSInfo.GPSVersionID                    Byte        4  2.2.0.0
Exif.GPSInfo.GPSLatitudeRef                  Ascii       2  North
Exif.GPSInfo.GPSLatitude                     Rational    3  51 deg 23' 13"
Exif.GPSInfo.GPSLongitudeRef                 Ascii       2  West
Exif.GPSInfo.GPSLongitude                    Rational    3  0 deg 44' 27"
Exif.GPSInfo.GPSAltitudeRef                  Byte        1  Above sea level
Exif.GPSInfo.GPSAltitude                     Rational    1  104.2 m
Exif.GPSInfo.GPSTimeStamp                    Rational    3  10:34:11
Exif.GPSInfo.GPSMapDatum                     Ascii       7  WGS-84
Exif.GPSInfo.GPSProcessingMethod             Undefined  18  charset=Ascii HYBRID-FIX
Exif.GPSInfo.GPSAreaInformation              Undefined  23  Surrey, England
Exif.GPSInfo.GPSDateStamp                    Ascii      20  2020:02:02 10:34:11
""","""0x9286 Photo        UserComment                 Undefined  12  charset=Unicode AB
0x0000 GPSInfo      GPSVersionID                Byte        4  2 2 0 0
0x0001 GPSInfo      GPSLatitudeRef              Ascii       2  N
0x0002 GPSInfo      GPSLatitude                 Rational    3  51/1 23/1 13/1
0x0003 GPSInfo      GPSLongitudeRef             Ascii       2  W
0x0004 GPSInfo      GPSLongitude                Rational    3  0/1 44/1 27/1
0x0005 GPSInfo      GPSAltitudeRef              Byte        1  0
0x0006 GPSInfo      GPSAltitude                 Rational    1  10420/100
0x0007 GPSInfo      GPSTimeStamp                Rational    3  10/1 34/1 11/1
0x0012 GPSInfo      GPSMapDatum                 Ascii       7  WGS-84
0x001b GPSInfo      GPSProcessingMethod         Undefined  18  charset=Ascii HYBRID-FIX
0x001c GPSInfo      GPSAreaInformation          Undefined  23  Surrey, England
0x001d GPSInfo      GPSDateStamp                Ascii      20  2020:02:02 10:34:11
""","","""Exif.Photo.UserComment                       Undefined  25  charset=Ascii how now brown cow
""","""0x9286 Photo        UserComment                 Undefined  25  charset=Ascii how now brown cow
""","","""Exif.Photo.UserComment                       Undefined  12  charset=Unicode AB
""","""0x9286 Photo        UserComment                 Undefined  12  charset=Unicode AB
""","","""Exif.Photo.UserComment                       Undefined  19  hello world
""","""0x9286 Photo        UserComment                 Undefined  19  hello world
""","","","""Exif.GPSInfo.GPSProcessingMethod             Undefined  18  charset=Unicode Robin
Exif.GPSInfo.GPSAreaInformation              Undefined  24  charset=Ascii area information
""","""0x001b GPSInfo      GPSProcessingMethod         Undefined  18  charset=Unicode Robin
0x001c GPSInfo      GPSAreaInformation          Undefined  24  charset=Ascii area information
"""
             ]
    stderr = [""]*len(commands)
    retval = [ 0]*len(commands)

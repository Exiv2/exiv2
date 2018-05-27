# -*- coding: utf-8 -*-

import system_tests


class CheckShutterInPentax(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1242"
    filename = "$data_path/Reagan.jp2"

    commands = ["$exiv2 -pa $filename"]

    stdout = [r"""Exif.Image.ImageDescription                  Ascii     403  040621-N-6536T-062
USS Ronald Reagan (CVN 76), June 21, 2004 -  USS Ronald Reagan (CVN 76) sails through the Straits of Magellan on its way to the Pacific Ocean. The NavyÃ­s newest aircraft carrier is underway circumnavigating South America in transit to its new homeport of San Diego. U.S. Navy photo by PhotographerÃ­s Mate 3rd Class (AW) Elizabeth Thompson. (RELEASE)
                               
Exif.Image.Make                              Ascii      18  NIKON CORPORATION
Exif.Image.Model                             Ascii      10  NIKON D1X
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  300
Exif.Image.YResolution                       Rational    1  300
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.Software                          Ascii      40  Adobe Photoshop Elements 12.0 Macintosh
Exif.Image.DateTime                          Ascii      20  2016:09:13 11:58:16
Exif.Image.Artist                            Ascii      34  PhotographerÃ­s Mate 3rd Class (A
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.ExifTag                           Long        1  712
Exif.Photo.ExposureTime                      Rational    1  1/125 s
Exif.Photo.FNumber                           Rational    1  F5
Exif.Photo.ExposureProgram                   Short       1  Manual
Exif.Photo.ExifVersion                       Undefined   4  2.20
Exif.Photo.DateTimeOriginal                  Ascii      20  2004:06:21 23:37:53
Exif.Photo.DateTimeDigitized                 Ascii      20  2004:06:21 23:37:53
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.ShutterSpeedValue                 SRational   1  1/125 s
Exif.Photo.ApertureValue                     Rational    1  F5
Exif.Photo.ExposureBiasValue                 SRational   1  +1/3 EV
Exif.Photo.MaxApertureValue                  Rational    1  F2.8
Exif.Photo.MeteringMode                      Short       1  Center weighted average
Exif.Photo.LightSource                       Short       1  Cloudy weather
Exif.Photo.Flash                             Short       1  No flash
Exif.Photo.FocalLength                       Rational    1  42.0 mm
Exif.Photo.SubSecTime                        Ascii       3  06
Exif.Photo.SubSecTimeOriginal                Ascii       3  06
Exif.Photo.SubSecTimeDigitized               Ascii       3  06
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ColorSpace                        Short       1  Uncalibrated
Exif.Photo.PixelXDimension                   Short       1  200
Exif.Photo.PixelYDimension                   Short       1  130
Exif.Photo.SensingMethod                     Short       1  One-chip color area
Exif.Photo.FileSource                        Undefined   1  Digital still camera
Exif.Photo.SceneType                         Undefined   1  Directly photographed
Exif.Photo.CustomRendered                    Short       1  Normal process
Exif.Photo.ExposureMode                      Short       1  Manual
Exif.Photo.WhiteBalance                      Short       1  Manual
Exif.Photo.DigitalZoomRatio                  Rational    1  1.0
Exif.Photo.FocalLengthIn35mmFilm             Short       1  63.0 mm
Exif.Photo.SceneCaptureType                  Short       1  Standard
Exif.Photo.GainControl                       Short       1  None
Exif.Photo.Contrast                          Short       1  Normal
Exif.Photo.Saturation                        Short       1  Normal
Exif.Photo.Sharpness                         Short       1  Normal
Exif.Photo.SubjectDistanceRange              Short       1  Unknown
Exif.Photo.ImageUniqueID                     Ascii      33  127c1377b054a3f65bf2754ebb24e7f2
Exif.Image.GPSTag                            Long        1  1300
Exif.GPSInfo.GPSVersionID                    Byte        4  2.2.0.0
Iptc.Application2.RecordVersion              Short       1  2
Iptc.Application2.ObjectName                 String     18  040621-N-6536T-062
Iptc.Application2.Urgency                    String      1  5
Iptc.Application2.Category                   String      1  N
Iptc.Application2.SuppCategory               String     12  703-614-9154
Iptc.Application2.SuppCategory               String     23  navyvisualnews@navy.mil
Iptc.Application2.SuppCategory               String     11  UNCLASSFIED
Iptc.Application2.Keywords                   String     13  ronald reagan
Iptc.Application2.Keywords                   String      6  reagan
Iptc.Application2.Keywords                   String      6  cvn 76
Iptc.Application2.Keywords                   String      6  cvn-76
Iptc.Application2.Keywords                   String     18  straights magellan
Iptc.Application2.Keywords                   String      8  magellan
Iptc.Application2.Keywords                   String      7  carrier
Iptc.Application2.Keywords                   String     12  nimitz-class
Iptc.Application2.Keywords                   String      4  ship
Iptc.Application2.Keywords                   String      8  underway
Iptc.Application2.SpecialInstructions        String     49  Credit as U.S. Navy photo by Elizabeth Thompson. 
Iptc.Application2.DateCreated                Date        8  2004-06-21
Iptc.Application2.TimeCreated                Time       11  00:00:00+00:00
Iptc.Application2.DigitizationDate           Date        8  2004-06-21
Iptc.Application2.DigitizationTime           Time       11  23:37:53-04:00
Iptc.Application2.Byline                     String     31  Photographerís Mate 3rd Class (
Iptc.Application2.BylineTitle                String     21  U.S Navy Photographer
Iptc.Application2.City                       String     19  Straits of Magellan
Iptc.Application2.CountryName                String     13  South America
Iptc.Application2.Credit                     String      8  U.S Navy
Iptc.Application2.Source                     String     24  Navy Visual News Service
Iptc.Application2.Caption                    String    400  040621-N-6536T-062
USS Ronald Reagan (CVN 76), June 21, 2004 -  USS Ronald Reagan (CVN 76) sails through the Straits of Magellan on its way to the Pacific Ocean. The Navyís newest aircraft carrier is underway circumnavigating South America in transit to its new homeport of San Diego. U.S. Navy photo by Photographerís Mate 3rd Class (AW) Elizabeth Thompson. (RELEASE)
                               
Iptc.Application2.Writer                     String      9  Dir. NVNS
Xmp.xmp.ModifyDate                           XmpText    25  2016-09-13T11:58:16+01:00
Xmp.xmp.CreateDate                           XmpText    25  2004-06-21T23:37:53+01:00
Xmp.xmp.MetadataDate                         XmpText    25  2016-09-13T11:58:16+01:00
Xmp.xmp.CreatorTool                          XmpText    38  Adobe Photoshop Elements 6.0 Macintosh
Xmp.photoshop.Instructions                   XmpText    49  Credit as U.S. Navy photo by Elizabeth Thompson. 
Xmp.photoshop.CaptionWriter                  XmpText     9  Dir. NVNS
Xmp.photoshop.Urgency                        XmpText     1  5
Xmp.photoshop.City                           XmpText    19  Straits of Magellan
Xmp.photoshop.Category                       XmpText     1  N
Xmp.photoshop.Country                        XmpText    13  South America
Xmp.photoshop.Credit                         XmpText     8  U.S Navy
Xmp.photoshop.AuthorsPosition                XmpText    21  U.S Navy Photographer
Xmp.photoshop.DateCreated                    XmpText    10  2004-06-21
Xmp.photoshop.Source                         XmpText    24  Navy Visual News Service
Xmp.photoshop.LegacyIPTCDigest               XmpText    32  977177A6C759A2BBD07317E3D5921073
Xmp.photoshop.ColorMode                      XmpText     1  3
Xmp.photoshop.ICCProfile                     XmpText    16  Adobe RGB (1998)
Xmp.photoshop.SupplementalCategories         XmpBag      3  703-614-9154, navyvisualnews@navy.mil, UNCLASSFIED
Xmp.xmpMM.InstanceID                         XmpText    40  xmp.iid:F77F117407206811822A8C00775B3FDC
Xmp.xmpMM.DocumentID                         XmpText    37  uuid:D6CBDC1D8DF2E511BA6BFBE914561F6D
Xmp.xmpMM.OriginalDocumentID                 XmpText    37  uuid:D6CBDC1D8DF2E511BA6BFBE914561F6D
Xmp.xmpMM.DerivedFrom                        XmpText     0  type="Struct"
Xmp.xmpMM.DerivedFrom/stRef:instanceID       XmpText    41  uuid:ec11a6b0-cc13-11d8-9c21-fa22e28297f6
Xmp.xmpMM.DerivedFrom/stRef:documentID       XmpText    58  adobe:docid:photoshop:1c90e091-c489-11d8-ad7d-b4c1b2598b09
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[1]/stEvt:instanceID        XmpText    40  xmp.iid:F77F117407206811822A8C00775B3FDC
Xmp.xmpMM.History[1]/stEvt:when              XmpText    25  2016-09-13T11:58:16+01:00
Xmp.xmpMM.History[1]/stEvt:softwareAgent     XmpText    39  Adobe Photoshop Elements 12.0 Macintosh
Xmp.xmpMM.History[1]/stEvt:changed           XmpText     1  /
Xmp.dc.format                                XmpText    10  image/jpeg
Xmp.dc.description                           LangAlt     1  lang="x-default" 040621-N-6536T-062
USS Ronald Reagan (CVN 76), June 21, 2004 -  USS Ronald Reagan (CVN 76) sails through the Straits of Magellan on its way to the Pacific Ocean. The NavyÃ­s newest aircraft carrier is underway circumnavigating South America in transit to its new homeport of San Diego. U.S. Navy photo by PhotographerÃ­s Mate 3rd Class (AW) Elizabeth Thompson. (RELEASE)
                               
Xmp.dc.title                                 LangAlt     1  lang="x-default" 040621-N-6536T-062
Xmp.dc.creator                               XmpSeq      1  PhotographerÃ­s Mate 3rd Class (A
Xmp.dc.subject                               XmpBag     10  ronald reagan, reagan, cvn 76, cvn-76, straights magellan, magellan, carrier, nimitz-class, ship, underway
Xmp.xmpRights.Marked                         XmpText     5  False
"""
    ]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """
        The metadata in this file contain badly encoded strings with linefeeds
        only resulting in un-comparable strings. Here we check line by line
        instead, which removes the nasty line ending problems.
        """
        for i, (got_line, expected_line) in \
            enumerate(zip(got_stdout.splitlines(), expected_stdout.splitlines())):
            self.assertEqual(
                got_line, expected_line, msg="error in line {:d}".format(i)
            )

    stderr = [""]
    retval = [0]

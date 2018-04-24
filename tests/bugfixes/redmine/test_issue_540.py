# -*- coding: utf-8 -*-

import system_tests


class PrettyPrintXmp(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/540"

    filename = "$data_path/exiv2-bug540.jpg"
    commands = ["$exiv2 -u -px $filename"]
    stdout = ["""Xmp.dc.creator                               XmpSeq      1  Ian Britton
Xmp.dc.description                           LangAlt     1  lang="x-default" Communications
Xmp.dc.rights                                LangAlt     1  lang="x-default" ian Britton - FreeFoto.com
Xmp.dc.source                                XmpText    12  FreeFoto.com
Xmp.dc.subject                               XmpBag      1  Communications
Xmp.dc.title                                 LangAlt     1  lang="x-default" Communications
Xmp.exif.ApertureValue                       XmpText     3  F16
Xmp.exif.BrightnessValue                     XmpText     8  0.260156
Xmp.exif.ColorSpace                          XmpText     1  sRGB
Xmp.exif.DateTimeOriginal                    XmpText    20  2002:07:13 15:58:28
Xmp.exif.ExifVersion                         XmpText     4  2.00
Xmp.exif.ExposureBiasValue                   XmpText     6  -13/20 EV
Xmp.exif.ExposureProgram                     XmpText     1  Shutter priority
Xmp.exif.FNumber                             XmpText     3  F0.6
Xmp.exif.FileSource                          XmpText     1  (0)
Xmp.exif.FlashpixVersion                     XmpText     4  1.00
Xmp.exif.FocalLength                         XmpText     3  0.0 mm
Xmp.exif.FocalPlaneResolutionUnit            XmpText     1  inch
Xmp.exif.FocalPlaneXResolution               XmpText     8  12.0508
Xmp.exif.FocalPlaneYResolution               XmpText     8  12.0508
Xmp.exif.GPSLatitude                         XmpText    13  54,59.380000N
Xmp.exif.GPSLongitude                        XmpText    12  1,54.850000W
Xmp.exif.GPSMapDatum                         XmpText     5  WGS84
Xmp.exif.GPSTimeStamp                        XmpText    20  2002:07:13 14:58:24
Xmp.exif.GPSVersionID                        XmpText     7  2.0.0.0
Xmp.exif.ISOSpeedRatings                     XmpSeq      1  0
Xmp.exif.MeteringMode                        XmpText     1  Multi-segment
Xmp.exif.PixelXDimension                     XmpText     4  2400
Xmp.exif.PixelYDimension                     XmpText     4  1600
Xmp.exif.SceneType                           XmpText     1  (0)
Xmp.exif.SensingMethod                       XmpText     1  One-chip color area
Xmp.exif.ShutterSpeedValue                   XmpText    10  1/724 s
Xmp.pdf.Keywords                             XmpText    14  Communications
Xmp.photoshop.AuthorsPosition                XmpText    12  Photographer
Xmp.photoshop.CaptionWriter                  XmpText    11  Ian Britton
Xmp.photoshop.Category                       XmpText     3  BUS
Xmp.photoshop.City                           XmpText     1   
Xmp.photoshop.Country                        XmpText    14  Ubited Kingdom
Xmp.photoshop.Credit                         XmpText    11  Ian Britton
Xmp.photoshop.DateCreated                    XmpText    10  2002-06-20
Xmp.photoshop.Headline                       XmpText    14  Communications
Xmp.photoshop.State                          XmpText     1   
Xmp.photoshop.SupplementalCategories         XmpBag      1  Communications
Xmp.photoshop.Urgency                        XmpText     1  5
Xmp.tiff.Artist                              XmpText    11  Ian Britton
Xmp.tiff.BitsPerSample                       XmpSeq      1  8
Xmp.tiff.Compression                         XmpText     1  6
Xmp.tiff.Copyright                           LangAlt     1  lang="x-default" ian Britton - FreeFoto.com
Xmp.tiff.ImageDescription                    LangAlt     1  lang="x-default" Communications
Xmp.tiff.ImageLength                         XmpText     3  400
Xmp.tiff.ImageWidth                          XmpText     3  600
Xmp.tiff.Make                                XmpText     8  FUJIFILM
Xmp.tiff.Model                               XmpText    12  FinePixS1Pro
Xmp.tiff.Orientation                         XmpText     1  top, left
Xmp.tiff.ResolutionUnit                      XmpText     1  inch
Xmp.tiff.Software                            XmpText    19  Adobe Photoshop 7.0
Xmp.tiff.XResolution                         XmpText     5  300
Xmp.tiff.YCbCrPositioning                    XmpText     1  Co-sited
Xmp.tiff.YResolution                         XmpText     5  300
Xmp.xmp.CreateDate                           XmpText    20  2002-07-13T15:58:28Z
Xmp.xmp.ModifyDate                           XmpText    20  2002-07-19T13:28:10Z
Xmp.xmpBJ.JobRef                             XmpText     0  type="Bag"
Xmp.xmpBJ.JobRef[1]                          XmpText     0  type="Struct"
Xmp.xmpBJ.JobRef[1]/stJob:name               XmpText    12  Photographer
Xmp.xmpMM.DocumentID                         XmpText    58  adobe:docid:photoshop:84d4dba8-9b11-11d6-895d-c4d063a70fb0
Xmp.xmpRights.Marked                         XmpText     4  True
Xmp.xmpRights.WebStatement                   XmpText    16  www.freefoto.com
"""]
    stderr = [""]
    retval = [0]

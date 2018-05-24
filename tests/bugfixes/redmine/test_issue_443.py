# -*- coding: utf-8 -*-

import system_tests


class CorruptedIopDir(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/443"

    filename = "$data_path/exiv2-bug443.jpg"
    commands = ["$exiv2 -u -b -pt $filename"]
    stdout = ["""Exif.Image.Make                              Ascii      22  EASTMAN KODAK COMPANY
Exif.Image.Model                             Ascii      28  KODAK DX3600 DIGITAL CAMERA
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.XResolution                       Rational    1  170
Exif.Image.YResolution                       Rational    1  170
Exif.Image.ResolutionUnit                    Short       1  inch
Exif.Image.DateTime                          Ascii      20  2003:07:27 13:08:24
Exif.Image.YCbCrPositioning                  Short       1  Centered
Exif.Image.ExifTag                           Long        1  208
Exif.Photo.ExposureTime                      Rational    1  0.002 s
Exif.Photo.FNumber                           Rational    1  F3.4
Exif.Photo.ExposureProgram                   Short       1  Auto
Exif.Photo.ExifVersion                       Undefined   4  2.00
Exif.Photo.DateTimeOriginal                  Ascii      20  2003:07:27 16:22:28
Exif.Photo.DateTimeDigitized                 Ascii      20  2003:07:27 16:22:28
Exif.Photo.ComponentsConfiguration           Undefined   4  YCbCr
Exif.Photo.ShutterSpeedValue                 SRational   1  1/512 s
Exif.Photo.ApertureValue                     Rational    1  F3.4
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.MaxApertureValue                  Rational    1  F3.2
Exif.Photo.SubjectDistance                   Rational    1  1.10 m
Exif.Photo.MeteringMode                      Short       1  Average
Exif.Photo.LightSource                       Short       1  Unknown
Exif.Photo.Flash                             Short       1  No flash
Exif.Photo.FocalLength                       Rational    1  5.6 mm
Exif.Photo.MakerNote                         Undefined 872  75 68 75 48 48 48 49 73 68 88 51 54 48 48 32 32 3 1 0 0 8 7 176 4 211 7 7 27 16 22 28 37 0 0 0 0 0 0 75 1 173 0 0 0 0 0 0 3 89 49 0 0 89 49 0 0 176 48 0 0 218 48 0 0 0 2 0 0 110 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 96 126 1 0 0 0 1 0 128 52 1 0 0 0 0 0 100 0 100 0 0 0 32 0 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 166 165 166 173 138 113 169 170 194 108 58 12 167 171 177 186 101 73 84 106 181 163 83 20 171 170 177 179 43 26 52 81 172 195 86 26 167 171 173 175 76 12 52 165 165 104 69 76 113 106 85 141 11 16 48 141 125 116 114 103 19 22 72 69 8 65 125 107 110 104 104 96 22 53 76 31 32 100 120 85 95 94 91 89 66 76 69 43 45 81 57 85 85 77 75 74 193 181 193 189 157 124 186 200 219 130 91 15 198 189 192 210 104 100 104 131 196 180 108 42 190 191 189 187 55 34 65 84 197 209 113 73 188 197 190 203 70 18 58 187 199 123 86 107 147 113 121 148 11 26 65 162 145 137 136 137 32 32 109 59 9 96 119 126 129 126 132 125 35 83 97 34 38 142 143 100 110 119 116 121 83 101 97 44 56 94 76 108 108 104 103 102 0 0 0 0 0 0 0 0 96 126 1 0 0 0 1 0 128 52 1 0 195 132 1 0 0 0 1 0 184 44 1 0 0 0 0 0 0 0 0 0 0 0 0 0 96 126 1 0 0 0 1 0 128 52 1 0 0 0 1 0 0 0 1 0 0 0 1 0 0 96 125 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 147 0 25 26 0 0 82 124 254 255 0 0 141 0 0 0 0 0 0 0 0 0 0 176 77 0 0 0 0 0 0 0 0 0 192 7 0 0 0 0 0 0 128 0 0 0 192 17 0 0 96 4 0 0 0 0 0 0 51 179 0 0 124 3 0 0 0 0 0 0 0 0 0 0 0 0 20 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 244 50 0 0 6 0 0 0 72 250 164 1 230 253 129 255 221 0 16 0 177 3 79 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 0 176 48 1 0 0 0 2 1 0 0 27 24 0 0 175 1 0 0 232 0 0 0 0 0 0 0 55 0 0 0 218 48 0 0 0 0 0 0 0 0 0 0 100 0 89 49 1 0 0 0 2 0 0 0 200 6 0 0 175 1 0 0 134 0 0 0 81 2 0 0 55 0 0 0 89 49 0 0 0 0 0 0 0 0 0 0 9 61 129 37 0 0 0 0 60 1 0 0 0 0 0 0 1 0 1 0 29 0 33 0 0 0 100 20 40 255 29 47 195 49 227 48 38 49 51 48 0 0 0 0 0 0 0 0 0 0 16 16 15 15 15 0 0 0 0 0 0 0 0 0 10 10 10 10 100 10 10 10 100 100 100 10 10 100 100 0 0 0 0 0 0 100 0 0 0 0 0 0 10 10 10 10 100 10 10 10 100 100 100 10 29 100 100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 1 0 0 118 1 0 176 163 255 255 80 230 255 255 160 224 255 255 144 113 1 0 208 173 255 255 224 6 0 0 64 146 255 255 224 102 1 0 0 0 0 0 0 0 0 0 0 0 0 0
Exif.Photo.UserComment                       Undefined   1  
Exif.Photo.FlashpixVersion                   Undefined   4  1.01
Exif.Photo.ColorSpace                        Short       1  sRGB
Exif.Photo.PixelXDimension                   Long        1  1024
Exif.Photo.PixelYDimension                   Long        1  683
Exif.Photo.InteroperabilityTag               Long        1  1850
Exif.Photo.ExposureIndex                     Rational    1  1677721600/16777216
Exif.Photo.SensingMethod                     Short       1  One-chip color area
Exif.Photo.FileSource                        Undefined   1  Digital still camera
Exif.Photo.SceneType                         Undefined   1  Directly photographed
Exif.Thumbnail.Compression                   Short       1  JPEG (old-style)
Exif.Thumbnail.Orientation                   Short       1  top, left
Exif.Thumbnail.XResolution                   Rational    1  72
Exif.Thumbnail.YResolution                   Rational    1  72
Exif.Thumbnail.ResolutionUnit                Short       1  inch
Exif.Thumbnail.JPEGInterchangeFormat         Long        1  1628
Exif.Thumbnail.JPEGInterchangeFormatLength   Long        1  33779
"""]
    stderr = ["""Error: Directory Iop with 5233 entries considered invalid; not read.
"""]
    retval = [0]

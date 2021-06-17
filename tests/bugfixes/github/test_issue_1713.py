# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class WebPImageGetHeaderOffset(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1713
    """
    url = "https://github.com/Exiv2/exiv2/issues/1713"

    filename = path("$data_path/issue_1713_poc.xmp")
    commands = ["$exiv2 -Ph $filename"]
    stdout = [
"""  0000  00 00 01 68                                      ...h

  0000  00 00 00 d8                                      ....

  0000  00 01                                            ..

  0000  00 0a fc 80 00 00 27 10                          ......'.

  0000  00 02                                            ..

  0000  32 30 30 35 3a 30 39 3a 30 37 20 32 33 3a 30 39  2005:09:07 23:09
  0010  3a 35 31 00                                      :51.

  0000  58 4d 50 46 69 6c 65 73 20 42 6c 75 65 53 71 75  XMPFiles BlueSqu
  0010  61 72 65 20 74 65 73 74 20 66 69 6c 65 2c 20 63  are test file, c
  0020  72 65 61 74 65 64 20 69 6e 20 50 68 6f 74 6f 73  reated in Photos
  0030  68 6f 70 20 43 53 32 2c 20 73 61 76 65 64 20 61  hop CS2, saved a
  0040  73 20 2e 70 73 64 2c 20 2e 6a 70 67 2c 20 61 6e  s .psd, .jpg, an
  0050  64 20 2e 74 69 66 2e 00                          d .tif..

  0000  4e 69 6b 6f 6e 00                                Nikon.

  0000  00 01                                            ..

  0000  42 6c 75 65 20 53 71 75 61 72 65 20 54 65 73 74  Blue Square Test
  0010  20 46 69 6c 65 20 2d 20 2e 6a 70 67               File - .jpg

  0000  1b 25 47                                         .%G

  0000  58 4d 50                                         XMP

  0000  42 6c 75 65 20 53 71 75 61 72 65                 Blue Square

  0000  74 65 73 74 20 66 69 6c 65                       test file

  0000  50 68 6f 74 6f 73 68 6f 70                       Photoshop

  0000  2e 6a 70 67                                      .jpg

  0000  32 30 30 35 30 39 30 37                          20050907

  0000  58 4d 50 46 69 6c 65 73 20 42 6c 75 65 53 71 75  XMPFiles BlueSqu
  0010  61 72 65 20 74 65 73 74 20 66 69 6c 65 2c 20 63  are test file, c
  0020  72 65 61 74 65 64 20 69 6e 20 50 68 6f 74 6f 73  reated in Photos
  0030  68 6f 70 20 43 53 32 2c 20 73 61 76 65 64 20 61  hop CS2, saved a
  0040  73 20 2e 70 73 64 2c 20 2e 6a 70 67 2c 20 61 6e  s .psd, .jpg, an
  0050  64 20 2e 74 69 66 2e                             d .tif.

"""]
    stderr = [
"""Warning: Failed to convert Xmp.xmp.CreateDate to Exif.Photo.DateTimeDigitized (Day is out of range)
Exiv2 exception in print action for file $filename:
Xmpdatum::copy: Not supported
"""
]
    retval = [1]

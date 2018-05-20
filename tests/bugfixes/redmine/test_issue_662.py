# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class UnicodeEncodingOfExifUserCommentTag(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/662"

    filename = "$data_path/exiv2-empty_copy.jpg"
    commands = [
        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Ascii An ascii comment" $filename""",
        "$exiv2 -u -PEnh $filename",

        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Ascii A\\nnewline" $filename""",
        "$exiv2 -u -PEnh $filename",

        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode A Unicode comment" $filename""",
        "$exiv2 -u -PEnh $filename",

        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode \\u01c4" $filename""",
        "$exiv2 -u -PEnh $filename",

        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode A\\u01c4C" $filename""",
        "$exiv2 -u -PEnh $filename",

        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode With\\nNewline" $filename""",
        "$exiv2 -u -PEnh $filename",

        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode With\\tTab" $filename""",
        "$exiv2 -u -PEnh $filename",

        #Test invalid escape sequences
        """$exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode \\ugggg" $filename""",
        "$exiv2 -u -PEnh $filename",
    ]
    stdout = [
        "",
        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  41 53 43 49 49 00 00 00 41 6e 20 61 73 63 69 69  ASCII...An ascii
  0010  20 63 6f 6d 6d 65 6e 74                           comment

""",
        "",
        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  41 53 43 49 49 00 00 00 41 0a 6e 65 77 6c 69 6e  ASCII...A.newlin
  0010  65                                               e

""",
        "",
        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  55 4e 49 43 4f 44 45 00 41 00 20 00 55 00 6e 00  UNICODE.A. .U.n.
  0010  69 00 63 00 6f 00 64 00 65 00 20 00 63 00 6f 00  i.c.o.d.e. .c.o.
  0020  6d 00 6d 00 65 00 6e 00 74 00                    m.m.e.n.t.

""",
        "",

        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  55 4e 49 43 4f 44 45 00 c4 01                    UNICODE...

""",
        "",
"""ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  55 4e 49 43 4f 44 45 00 41 00 c4 01 43 00        UNICODE.A...C.

""",
        "",
        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  55 4e 49 43 4f 44 45 00 57 00 69 00 74 00 68 00  UNICODE.W.i.t.h.
  0010  0a 00 4e 00 65 00 77 00 6c 00 69 00 6e 00 65 00  ..N.e.w.l.i.n.e.

""",
        "",
        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  55 4e 49 43 4f 44 45 00 57 00 69 00 74 00 68 00  UNICODE.W.i.t.h.
  0010  09 00 54 00 61 00 62 00                          ..T.a.b.

""",
        "",
        """ExifTag                    
  0000  1a 00 00 00                                      ....

UserComment                
  0000  55 4e 49 43 4f 44 45 00 5c 00 75 00 67 00 67 00  UNICODE.\.u.g.g.
  0010  67 00 67 00                                      g.g.

"""
    ]
    stderr = [""] * 16
    retval = [0] * 16

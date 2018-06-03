# -*- coding: utf-8 -*-

import system_tests

class CheckMWG_KW_Schema(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1040"

    filename = system_tests.path("$data_path/exiv2-bug1040.jpg")
    commands = [ "$exiv2 -px $filename" ]

    stdout = [ """Xmp.mwg-kw.Keywords                          XmpText     0  type="Struct"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy         XmpText     0  type="Bag"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[1]      XmpText     0  type="Struct"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[1]/mwg-kw:Keyword XmpText     3  cat
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[1]/mwg-kw:Children XmpText     0  type="Bag"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[1]/mwg-kw:Children[1] XmpText     0  type="Struct"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[1]/mwg-kw:Children[1]/mwg-kw:Keyword XmpText     7  Siamese
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[2]      XmpText     0  type="Struct"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[2]/mwg-kw:Keyword XmpText     7  Felidae
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[2]/mwg-kw:Children XmpText     0  type="Bag"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[2]/mwg-kw:Children[1] XmpText     0  type="Struct"
Xmp.mwg-kw.Keywords/mwg-kw:Hierarchy[2]/mwg-kw:Children[1]/mwg-kw:Keyword XmpText     6  Jaguar
"""
    ]
    stderr = [""]
    retval = [0]

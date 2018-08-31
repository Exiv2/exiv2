# -*- coding: utf-8 -*-

import system_tests

@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class CheckXmlLang(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1058"

    filename = system_tests.path("$data_path/exiv2-empty_copy.jpg")
    commands = [
        ## Add titles in 2 languages and one default
        """$exiv2 -M"set Xmp.dc.title lang=de-DE GERMAN"  $filename""",
        """$exiv2 -M"set Xmp.dc.title lang=en-GB BRITISH" $filename""",
        """$exiv2 -M"set Xmp.dc.title Everybody else"     $filename""",
        """$exiv2 -px                                     $filename""",
        ## Remove languages, test case for the language
        """$exiv2 -M"set Xmp.dc.title lang=DE-de german"  $filename""",
        """$exiv2 -M"set Xmp.dc.title lang=EN-gb"         $filename""",
        """$exiv2 -M"set Xmp.dc.title"                    $filename""",
        """$exiv2 -px                                     $filename""",
    ]

    stdout = [
        "",
        "",
        "",
        """Xmp.dc.title                                 LangAlt     3  lang="x-default" Everybody else, lang="en-GB" BRITISH, lang="de-DE" GERMAN
""",
        "",
        "",
        "",
        """Xmp.dc.title                                 LangAlt     1  lang="de-DE" german
""",
             ]

    stderr = [""] * len(commands)
    retval = [0] * len(commands)

# -*- coding: utf-8 -*-

import system_tests

@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class CheckXmpLangAltValues(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/1481"

# Python unittest is filtering out empty pairs of "" and flags up an error if 
# a mismatched number of quotes is used inside the commands[] (see
# https://docs.python.org/3/library/shlex.html#parsing-rules).
#
# This means that some of the tests in the github issue cannot be run.

    langAltValue = [
        # 1. No language value
        """lang= test1-1""",
        """lang=\" test1-2""",

        # 2. Empty language value
        """lang=\"\" test2""",
 
        # 3. Mismatched and/or incorrect positioning of quotation marks
        """lang=\"\"test3-1""",
        """lang=\"test3-2""",
        """lang=\"en-UK test3-3""",
        """lang=en-US\" test3-4""",
        """lang=test3-5\"""",
        """lang=test3-6\"\"""",
        
        # 4. Invalid characters in language part
        """lang=en-UK- test4-1""",
        """lang=en=UK test4-2""",
    ]

    filename = system_tests.path("$data_path/exiv2-empty_copy.jpg")

    commands = [
                # 1. No language value
                """$exiv2 -M"set Xmp.dc.title """ + langAltValue[0]  + """" $filename""",
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[1]  + """" $filename""",
                """$exiv2 -px                                               $filename""",
        
                # 2. Empty language value
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[2]  + """" $filename""",
#               """$exiv2 -px                                               $filename""",
        
                # 3. Mismatched and/or incorrect positioning of quotation marks
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[3]  + """" $filename""",
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[4]  + """" $filename""",
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[5]  + """" $filename""",
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[6]  + """" $filename""",
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[7]  + """" $filename""",
#               """$exiv2 -M"set Xmp.dc.title """ + langAltValue[8]  + """" $filename""",
#               """$exiv2 -px                                               $filename""",
                
                # 4. Invalid characters in language part
                """$exiv2 -M"set Xmp.dc.title """ + langAltValue[9]  + """" $filename""",
                """$exiv2 -M"set Xmp.dc.title """ + langAltValue[10] + """" $filename""",
                """$exiv2 -px                                               $filename"""
    ]

    stdout = [
              # 1. No language value
              "",
#             "",
              "",
              
              # 2. Empty language value
#             "",
#             "",
              
              # 3. Mismatched and/or incorrect positioning of quotation marks
#             "",
#             "",
#             "",
#             "",
#             "",
#             "",
#             "",
              
              # 4. Invalid characters in language part
              "",
              "",
              ""
    ]

    stderr = [
              # 1. No language value
              """$exiv2_modify_exception_message $filename:
$kerInvalidLangAltValue `""" + langAltValue[0]  + """'
""",
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[1]  + """'
# """,
              "",
                      
              # 2. Empty language value
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[2]  + """'
# """,
#             "",
        
              # 3. Mismatched and/or incorrect positioning of quotation marks
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[3]  + """'
# """,
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[4]  + """'
# """,
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[5]  + """'
# """,
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[6]  + """'
# """,
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[7]  + """'
# """,
#             """$exiv2_modify_exception_message $filename:
# $kerInvalidLangAltValue `""" + langAltValue[8]  + """'
# """,
#             "",
              
              # 4. Invalid characters in language part
              """$exiv2_modify_exception_message $filename:
$kerInvalidLangAltValue `""" + langAltValue[9]  + """'
""",
              """$exiv2_modify_exception_message $filename:
$kerInvalidLangAltValue `""" + langAltValue[10]  + """'
""",
              ""
    ]
        
    retval = [
              # 1. No language value
              1,
#             1,
              0,
        
              # 2. Empty language value
#             1,
#             0,
              
              # 3. Mismatched and/or incorrect positioning of quotation marks
#             1,
#             1,
#             1,
#             1,
#             1,
#             1,
#             0,
              
              # 4. Invalid characters in language part
              1,
              1,
              0
    ]


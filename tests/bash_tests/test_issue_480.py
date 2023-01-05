# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-bug480.jpg")
class LargeIptcTest(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/480"

    bug_file = "$data_path/exiv2-bug480_copy.jpg"
    imagemagick_file = "$data_path/imagemagick.png"

    commands = ["$largeiptc_test $bug_file $imagemagick_file"]

    stdout = ["""Reading 144766 bytes from $imagemagick_file
IPTC fields: 0
IPTC fields: 144775
IRB buffer : 144788
Comparing IPTC and IRB size... ok
Comparing IPTC and IRB data... ok
"""]
    stderr = [""]
    retval = [0]

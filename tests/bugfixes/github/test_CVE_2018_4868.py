# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/202"
    cve_url = "http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-4868"
    found_by = ["afl", "topsecLab", "xcainiao"]

    filename = "{data_path}/exiv2-memorymmap-error"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_message} """ + filename + """:
{kerCorruptedMetadata}
"""]
    retval = [1]

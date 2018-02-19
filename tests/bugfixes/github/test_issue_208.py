# -*- coding: utf-8 -*-

import system_tests


class CVE_2017_14857(system_tests.Case):

    filename = "{data_path}/2018-01-09-exiv2-crash-001.tiff"
    commands = ["{exiv2} " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
        """{exiv2_exception_msg} """ + filename + """:
{kerCorruptedMetadata}
"""]

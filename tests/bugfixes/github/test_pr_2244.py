# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path


@CopyTmpFiles("$data_path/test_issue_1180.exv")
class test_pr_2244(metaclass=CaseMeta):
    filename = path("$tmp_path/test_issue_1180.exv")
    commands = [
        "$exiv2 -Y 10000000000        $filename",
        "$exiv2 -Y -10000000000       $filename",
        "$exiv2 -O 10000000000        $filename",
        "$exiv2 -O -10000000000       $filename",
        "$exiv2 -D 1000000000000000   $filename",
        "$exiv2 -D -1000000000000000  $filename",
    ]
    stdout = ["", "", "", "", "", ""]
    stderr = [
        "Uncaught exception: year adjustment too high\n",
        "Uncaught exception: year adjustment too low\n",
        "Uncaught exception: month adjustment too high\n",
        "Uncaught exception: month adjustment too low\n",
        "Uncaught exception: day adjustment too high\n",
        "Uncaught exception: day adjustment too low\n",
    ]
    retval = [1, 1, 1, 1, 1, 1]

# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class test_issue_ghsa_mv9g_fxh2_m49m(metaclass=CaseMeta):

    filename = path("$data_path/test_issue_ghsa_mv9g_fxh2_m49m.crw")
    commands  = ["$exiv2 fi $filename"]
    stdout = [""]
    stderr = ["""Exiv2 exception in fixiso action for file $filename:
$kerCorruptedMetadata
"""]
    retval = [1]

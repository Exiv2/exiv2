# -*- coding: utf-8 -*-

import system_tests

class test_pr_1384(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1384"
    
    filename = "$data_path/pr_1384.exv"
    commands = ["$exiv2 --grep ColorMatrix/i $filename"]
    stderr = [""]
    stdout = ["""Exif.Image.ColorMatrix1                      SRational   9  10236/10000 -4532/10000 397/10000 -3403/10000 10623/10000 3223/10000 -242/10000 968/10000 8530/10000
Exif.Image.ColorMatrix2                      SRational   9  8695/10000 -2558/10000 -648/10000 -5015/10000 12711/10000 2575/10000 -1279/10000 2215/10000 7514/10000
"""
]
    retval = [0]

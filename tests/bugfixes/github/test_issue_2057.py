# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class PanasonicMakerPrintAccelerometerIntOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2057
    """
    url = "https://github.com/Exiv2/exiv2/issues/2057"

    filename = path("$data_path/issue_2057_poc1.exv")
    commands = ["$exiv2 --Print kyyvt --key Exif.CanonCs.LensType $filename"]
    stderr = [""]
    stdout = ["""Exif.CanonCs.LensType                        Short      61182  Canon RF 24-105mm F4L IS USM *OR* Canon RF 24-105mm F4-7.1 IS STM
"""]
    retval = [0]

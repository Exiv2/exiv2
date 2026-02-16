# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles


@CopyTmpFiles("$data_path/FurnaceCreekInn.jpg")
class jpg_iptcDataSegs(metaclass=CaseMeta):
    """
    test for the iptcDataSegs code in jpgimage.cpp
    """

    filename = "$tmp_path/FurnaceCreekInn.jpg"
    commands = ["$exiv2 -d I rm $filename"]
    retval = [0]
    stderr = [""]
    stdout = [""]

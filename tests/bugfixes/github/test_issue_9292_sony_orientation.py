# -*- coding: utf-8 -*-

import system_tests
import unittest

# test needs system_tests.BT.vv['enable_bmff']=1
bSkip = system_tests.BT.verbose_version().get("enable_bmff") != "1"
if bSkip:
    raise unittest.SkipTest("*** requires enable_bmff=1 ***")


###########################################################
# Sony ILCE-7CM2 HIF orientation = 1 (top-left)
###########################################################
class issue_9292_sony_orientation_1(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9292"
    filename = "$data_path/issue_9292_sony_1_top_left.HIF"

    if bSkip:
        commands = []
        retval = []
        stdin = []
        stderr = []
        stdout = []
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = [
            "$exiv2 -g SonyMisc3c.CameraOrientation $filename",
        ]
        retval = [0] * len(commands)
        stderr = [""] * len(commands)
        stdin = [""] * len(commands)
        stdout = [
            "Exif.SonyMisc3c.CameraOrientation            Byte        1  Horizontal (normal)\n",
        ]


###########################################################
# Sony ILCE-7CM2 HIF orientation = 3 (bottom-right, 180°)
###########################################################
class issue_9292_sony_orientation_3(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9292"
    filename = "$data_path/issue_9292_sony_3_bottom_right.HIF"

    if bSkip:
        commands = []
        retval = []
        stdin = []
        stderr = []
        stdout = []
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = [
            "$exiv2 -g SonyMisc3c.CameraOrientation $filename",
        ]
        retval = [0] * len(commands)
        stderr = [""] * len(commands)
        stdin = [""] * len(commands)
        stdout = [
            "Exif.SonyMisc3c.CameraOrientation            Byte        1  Rotate 180°\n",
        ]


###########################################################
# Sony ILCE-7CM2 HIF orientation = 6 (right-top, 90° CW)
###########################################################
class issue_9292_sony_orientation_6(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9292"
    filename = "$data_path/issue_9292_sony_6_right_top.HIF"

    if bSkip:
        commands = []
        retval = []
        stdin = []
        stderr = []
        stdout = []
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = [
            "$exiv2 -g SonyMisc3c.CameraOrientation $filename",
        ]
        retval = [0] * len(commands)
        stderr = [""] * len(commands)
        stdin = [""] * len(commands)
        stdout = [
            "Exif.SonyMisc3c.CameraOrientation            Byte        1  Rotate 90° CW\n",
        ]


###########################################################
# Sony ILCE-7CM2 HIF orientation = 8 (left-bottom, 270° CW)
###########################################################
class issue_9292_sony_orientation_8(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9292"
    filename = "$data_path/issue_9292_sony_8_left-bottom.HIF"

    if bSkip:
        commands = []
        retval = []
        stdin = []
        stderr = []
        stdout = []
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = [
            "$exiv2 -g SonyMisc3c.CameraOrientation $filename",
        ]
        retval = [0] * len(commands)
        stderr = [""] * len(commands)
        stdin = [""] * len(commands)
        stdout = [
            "Exif.SonyMisc3c.CameraOrientation            Byte        1  Rotate 270° CW\n",
        ]

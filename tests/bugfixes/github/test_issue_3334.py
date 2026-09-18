# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path


@CopyTmpFiles("$data_path/exiv2-empty.jpg")
class test_issue_3334Test(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/3334

    Insert (-i) has to keep every value of a repeatable IPTC dataset instead of
    collapsing them onto the target's first matching entry.
    """

    url = "https://github.com/Exiv2/exiv2/issues/3334"

    filename = path("$tmp_path/exiv2-empty.jpg")
    commands = [
        '$exiv2 -M"add Iptc.Application2.Keywords one"   $filename',
        '$exiv2 -M"add Iptc.Application2.Keywords two"   $filename',
        '$exiv2 -M"add Iptc.Application2.Keywords three" $filename',
        "$exiv2 -ea --force $filename",
        "$exiv2 -ia $filename",
        "$exiv2 -PI -g Keywords $filename",
    ]
    stdout = [
        "",
        "",
        "",
        "",
        "",
        """Iptc.Application2.Keywords                   String      3  one
Iptc.Application2.Keywords                   String      3  two
Iptc.Application2.Keywords                   String      5  three
""",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

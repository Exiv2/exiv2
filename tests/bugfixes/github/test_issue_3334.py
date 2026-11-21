# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, DeleteFiles, path


@DeleteFiles("$exvname")
@CopyFiles("$data_path/exiv2-empty.jpg")
class test_issue_3334Test(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/3334

    Insert (-i) has to keep every value of a repeatable IPTC dataset instead of
    collapsing them onto the target's first matching entry.
    """

    url = "https://github.com/Exiv2/exiv2/issues/3334"

    # Work on a private copy and remove the extracted sidecar afterwards, so that the
    # test neither shares nor leaves behind files that other tests use in test/tmp.
    filename_common = path("$data_path/exiv2-empty_copy")
    filename = "$filename_common.jpg"
    exvname = "$filename_common.exv"
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

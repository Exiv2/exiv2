# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path
import zlib

class tagListTest(metaclass=CaseMeta):
    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        lines    = got_stdout.split('\n')
        count    = len(lines);
        checksum = zlib.adler32(got_stdout.encode())
        new_got  = "%s,%s" % (count, checksum)
        return super().compare_stdout(i, command, new_got, expected_stdout)

    commands = ["$taglist --help",
                "$taglist all",
                "$taglist ALL",
                "$taglist Groups",
                "$taglist nothing",
               ]
    stdout =   ["8,198867862",
                "5426,1072295739",
                "5426,838787056",
                "107,430791180",
                "8,198867862",
               ]

    stderr = ["","","","","Unexpected argument nothing\n"]
    retval = [ 0,0, 0, 0, 2]

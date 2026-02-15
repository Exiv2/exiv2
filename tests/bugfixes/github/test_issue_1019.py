from system_tests import CaseMeta, path


class OverreadInCiffDirectoryReadDirectory(metaclass=CaseMeta):

    filename = path("$data_path/POC-file_issue_1019")
    commands = ["$exiv2 -pv $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerOffsetOutOfRange
"""
    ]
    retval = [1]

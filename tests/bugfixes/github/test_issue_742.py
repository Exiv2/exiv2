import system_tests


class ThrowsWhenSubBoxLengthIsNotGood(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/742"

    filename = system_tests.path("$data_path/issue_742_poc")
    commands = ["$exiv2 -pX $filename"]
    stdout = [""]
    stderr = ["""$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    retval = [1]

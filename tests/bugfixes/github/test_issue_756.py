import system_tests


class BufferOverReadInNikon1MakerNotePrint0x0088(
        metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/756"

    filename = system_tests.path(
        "$data_path/NikonMakerNotePrint0x088_overread"
    )
    commands = ["$exiv2 -pt --grep AFFocusPos $filename"]
    stdout = [
        """Exif.Nikon1.AFFocusPos                       Undefined   4  Invalid value; Center
"""
    ]
    stderr = [""]
    retval = [0]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors

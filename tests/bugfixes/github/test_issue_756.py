import system_tests


class BufferOverReadInNikon1MakerNotePrint0x0088(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/756"

    filename = system_tests.path("$data_path/NikonMakerNotePrint0x088_overread")
    commands = ["$exiv2 -q -pt --grep AFFocusPos $filename"]
    stderr = [
        f"""$exiv2_exception_message {filename}:
$kerFailedToReadImageData
"""
    ]
    retval = [1]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors

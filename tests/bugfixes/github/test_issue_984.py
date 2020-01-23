import system_tests


class XMPMetaDeleteNamespace(
        metaclass=system_tests.CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/984
    """
    url = "https://github.com/Exiv2/exiv2/issues/984"

    filename = system_tests.path(
        "$data_path/issue_94_poc3.pgf"
    )
    commands = ["$exiv2json $filename"]
    stdout = ["""Caught Exiv2 exception 'XMP Toolkit error 9: Fatal namespace map problem'
"""]
    stderr = ["""Warning: Removing 54 characters from the beginning of the XMP packet
"""]
    retval = [255]

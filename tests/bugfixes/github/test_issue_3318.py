# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path


@CopyTmpFiles("$data_path/exiv2-bug1044.tif")
class DeleteXmpFromTiff(metaclass=CaseMeta):
    """
    Regression test: "exiv2 -dx" must actually erase XMP from a TIFF.

    In a TIFF the XMP is stored in the Exif.Image.XMLPacket tag. Deleting it
    used to leave a stale packet cached in xmpData_, which the TIFF encoder
    wrote back on save, so the XMP survived "exiv2 -dx".

    https://github.com/Exiv2/exiv2/issues/3318
    """

    url = "https://github.com/Exiv2/exiv2/issues/3318"

    filename = path("$tmp_path/exiv2-bug1044.tif")
    commands = [
        # Add an XMP property to the TIFF.
        '$exiv2 -M"set Xmp.dc.subject MyCustomXmpValue" $filename',
        # It is present.
        "$exiv2 -px $filename",
        # Delete all XMP.
        "$exiv2 -dx $filename",
        # It must be gone (this printed the value again before the fix).
        "$exiv2 -px $filename",
    ]
    stdout = [
        "",
        "Xmp.dc.subject                               XmpBag      1  MyCustomXmpValue\n",
        "",
        "",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

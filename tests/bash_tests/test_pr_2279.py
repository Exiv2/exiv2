# -*- coding: utf-8 -*-

import system_tests

class TestExifIPTCXmpTagOutput(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/2279"
    
    # Test the exiv2 application's `--Print` command
    filename = "$data_path/Stonehenge.exv"
    commands = ["$exiv2 --Print xVgknldycsvt --key Exif.Image.Model $filename",
                "$exiv2 --Print xVgknldycsvt --key Iptc.Application2.Caption $filename",
                "$exiv2 --Print xVgknldycsvt --key Xmp.dc.description $filename"]
    stderr = [""] * len(commands)
    stdout = ["""0x0110 set Image        Exif.Image.Model                             Model                       Model                          The model name or model number of the equipment. This is the model name or number of the DSC, scanner, video digitizer or other equipment that generated the image. When the field is left blank, it is treated as unknown. Ascii      12  12  NIKON D5300  NIKON D5300
""",
"""0x0078 set Application2 Iptc.Application2.Caption                    Caption                     Caption                        A textual description of the object data. String     12  12  Classic View  Classic View
""",
"""0x0000 set dc           Xmp.dc.description                           description                 Description                    A textual description of the content of the resource. Multiple values may be present for different languages. LangAlt     1  29  lang="x-default" Classic View  lang="x-default" Classic View
"""
]
    retval = [0] * len(commands)

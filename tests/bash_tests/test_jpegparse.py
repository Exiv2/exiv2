# -*- coding: utf-8 -*-

import system_tests

class TestJpegParse(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/2874"
    
    # Test extraction of jpeg specific metadata
    filename = "$data_path/exiv2-fujifilm-finepix-s2pro.jpg"
    commands = ["$jpegparsetest $filename"]
    stderr = [""] * len(commands)
    stdout = ["""Number of color components: 3\nEncoding process: Baseline DCT, Huffman coding\n"""]
    retval = [0] * len(commands)

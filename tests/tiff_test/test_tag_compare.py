# -*- coding: utf-8 -*-

import system_tests


class OutputTagExtract(system_tests.Case):
    """
    Test whether exiv2 -pa $file and exiv2 -pS $file produces the same output.
    """

    def parse_pa(self, stdout):
        """
        Parse the output of exiv2 -pa $file, which looks like this:

        Exif.Image.NewSubfileType                    Long        1  Primary image

        into a list of dictionaries with the keys:
        tag: last word of the first column (here NewSubfileType)
        type: lowercase second column
        len: third column
        val: fourth column

        It is furthermore checked that the first column begins with 'Exif.Image'
        """
        data = []

        for line in stdout:
            tmp = line.split()

            exif, image, tag = tmp[0].split('.')
            self.assertEquals(exif, "Exif")
            self.assertEquals(image, "Image")

            data.append({
                "tag": tag,
                "type": tmp[1].lower(),
                "len": int(tmp[2]),
                "val": " ".join(tmp[3:])
            })

        return data

    def parse_pS(self, stdout):
        """
        Parse the output of exiv2 -pS $file, which looks like this:

        STRUCTURE OF TIFF FILE (II): $file
        address |    tag                              |      type |    count |    offset | value
        254 | 0x00fe NewSubfileType               |      LONG |        1 |           | 0
         ...
        END $file

        into a list of dictionaries with the following keys:
        tag: the string after the hex number in the second column
        type: lowercase third column
        len: fourth column
        val: fifth column

        The first two lines and the last line are ignored, as they contain
        explanatory output.
        """
        data = []

        for i, line in enumerate(stdout):
            if i < 2 or i == len(stdout) - 1:
                continue

            tmp = line.split(" | ")
            data.append({
                "tag": tmp[1].split()[1],
                "type": tmp[2].replace(' ', '').lower(),
                "len": int(tmp[3].replace(' ', '')),
                "val": tmp[5]
            })

        return data

    def compare_pS_pa(self):
        """
        Compares the output from self.parse_pa() and self.parse_pS() (saved in
        self.pa_data & self.pS_data respectively).
        All dictionaries in the lists are compared for equality for the keys
        tag, len and type but only some for val. This is due to differently
        processed output (exiv2 -pa produces more readable output,
        e.g. compression is written in words and not as a number as it is by
        exiv2 -pS)
        """
        for pa_elem, pS_elem in zip(self.pa_data, self.pS_data):
            for key in ["tag", "type", "len"]:
                self.assertEquals(pa_elem[key], pS_elem[key])

            if pa_elem["tag"] in [
                    "ImageWidth", "ImageLength", "BitsPerSample",
                    "DocumentName", "ImageDescription", "StripOffsets",
                    "SamplesPerPixel", "StripByteCounts", "PlanarConfiguration"]:
                self.assertEquals(pa_elem["val"], pS_elem["val"])

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        super().compare_stdout(i, command, got_stdout, expected_stdout)

        if '-pa' in command:
            self.pa_data = self.parse_pa(got_stdout.splitlines())
        if '-pS' in command:
            self.pS_data = self.parse_pS(got_stdout.splitlines())

        if i == 1:
            self.compare_pS_pa()

    commands = [
        "{exiv2} %s {data_path}/mini9.tif" % (opt) for opt in ["-pa", "-pS"]
    ]

    stderr = [""] * 2
    retval = [0] * 2
    stdout = [
        """Exif.Image.NewSubfileType                    Long        1  Primary image
Exif.Image.ImageWidth                        Short       1  9
Exif.Image.ImageLength                       Short       1  9
Exif.Image.BitsPerSample                     Short       3  8 8 8
Exif.Image.Compression                       Short       1  Uncompressed
Exif.Image.PhotometricInterpretation         Short       1  RGB
Exif.Image.DocumentName                      Ascii      24  /home/ahuggel/mini9.tif
Exif.Image.ImageDescription                  Ascii      18  Created with GIMP
Exif.Image.StripOffsets                      Long        1  8
Exif.Image.Orientation                       Short       1  top, left
Exif.Image.SamplesPerPixel                   Short       1  3
Exif.Image.RowsPerStrip                      Short       1  64
Exif.Image.StripByteCounts                   Long        1  243
Exif.Image.XResolution                       Rational    1  72
Exif.Image.YResolution                       Rational    1  72
Exif.Image.PlanarConfiguration               Short       1  1
Exif.Image.ResolutionUnit                    Short       1  inch
""",
        """STRUCTURE OF TIFF FILE (II): {data_path}/mini9.tif
 address |    tag                              |      type |    count |    offset | value
     254 | 0x00fe NewSubfileType               |      LONG |        1 |           | 0
     266 | 0x0100 ImageWidth                   |     SHORT |        1 |           | 9
     278 | 0x0101 ImageLength                  |     SHORT |        1 |           | 9
     290 | 0x0102 BitsPerSample                |     SHORT |        3 |       462 | 8 8 8
     302 | 0x0103 Compression                  |     SHORT |        1 |           | 1
     314 | 0x0106 PhotometricInterpretation    |     SHORT |        1 |           | 2
     326 | 0x010d DocumentName                 |     ASCII |       24 |       468 | /home/ahuggel/mini9.tif
     338 | 0x010e ImageDescription             |     ASCII |       18 |       492 | Created with GIMP
     350 | 0x0111 StripOffsets                 |      LONG |        1 |           | 8
     362 | 0x0112 Orientation                  |     SHORT |        1 |           | 1
     374 | 0x0115 SamplesPerPixel              |     SHORT |        1 |           | 3
     386 | 0x0116 RowsPerStrip                 |     SHORT |        1 |           | 64
     398 | 0x0117 StripByteCounts              |      LONG |        1 |           | 243
     410 | 0x011a XResolution                  |  RATIONAL |        1 |       510 | 1207959552/16777216
     422 | 0x011b YResolution                  |  RATIONAL |        1 |       518 | 1207959552/16777216
     434 | 0x011c PlanarConfiguration          |     SHORT |        1 |           | 1
     446 | 0x0128 ResolutionUnit               |     SHORT |        1 |           | 2
END {data_path}/mini9.tif
"""]

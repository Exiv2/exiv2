import system_tests
import os
import shutil


# copy the example config file into current working directory
# and name it "exiv2.ini" on Win or ".exiv2" on other platforms
class TmpConfigFile(system_tests.FileDecoratorBase):
    def setUp_file_action(self, expanded_file_name):
        config_file_path = os.path.dirname(os.path.abspath(__file__))
        fname = os.path.basename(system_tests.BT.verbose_version().get('config_path'))
        return shutil.copyfile(expanded_file_name, os.path.join(config_file_path, fname))


@TmpConfigFile("$data_path/example_exiv2_config_file_for_lens_test")
class TestLensConfigFile(metaclass=system_tests.CaseMeta):
    """
    Simple test for the configuration file based lens name resolution
    """

    url = "https://github.com/Exiv2/exiv2/pull/1783"

    filenames = [
        "$data_path/Sigma_120-300_DG_OS_HSM_Sport_lens.exv",
        "$data_path/Sigma_14-24mm_F2.8_DG_HSM_A_for_EOS.exv",
        "$data_path/olympus-m.zuiko-12-45mm-f4-pro.exv",
        "$data_path/RAW_PENTAX_K100.exv",
        "$data_path/exiv2-bug1145a.exv",
        "$data_path/exiv2-bug1145b.exv",
    ]

    commands = [
        f"$exiv2 -pt -g LensIDNumber {filenames[0]}",
        f"$exiv2 -pt -g LensType {filenames[1]}",
        f"$exiv2 -pt -g LensType {filenames[2]}",
        f"$exiv2 -pt -g LensType {filenames[3]}",
        f"$exiv2 -pt -g LensID {filenames[4]}",
        f"$exiv2 -pt -g LensID {filenames[5]}",
    ]

    stderr = [""] * len(filenames)
    stdout = [
        "Exif.NikonLd3.LensIDNumber                   Byte        1  TEST CONFIGURATION FILE!\n",
        "Exif.CanonCs.LensType                        Short       1  TEST CONFIGURATION FILE!\n",
        "Exif.OlympusEq.LensType                      Byte        6  TEST CONFIGURATION FILE!\n",
        "Exif.Pentax.LensType                         Byte        2  TEST CONFIGURATION FILE!\n",
        "Exif.Sony1.LensID                            Long        1  TEST CONFIGURATION FILE!\n",
        "Exif.Sony1.LensID                            Long        1  TEST CONFIGURATION FILE!\n",
    ]
    retval = [0] * len(filenames)

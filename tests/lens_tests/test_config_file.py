import unittest
import system_tests
import os
import shutil


class TmpConfigFile(system_tests.FileDecoratorBase):
    def setUp_file_action(self, expanded_file_name):
        # get platform dependent path where exiv2 looks for the config file
        config_file_path = system_tests.BT.verbose_version()["config_path"]
        # if file already exist we skip this test.
        # in the CI this doesn't happen, but if a user executes the test suite
        # I don't want to override their config file or implement a lot of logic to take care of that scenario
        if os.path.isfile(config_file_path):
            raise unittest.SkipTest(
                "*** can not run test as pre-existing config file was found on system: {} ***".format(config_file_path)
            )

        return shutil.copyfile(expanded_file_name, config_file_path)


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

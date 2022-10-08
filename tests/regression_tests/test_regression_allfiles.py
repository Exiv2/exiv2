import os
import unittest

from system_tests import BT


def get_valid_files(data_dir):
    """return a list of files inside 'data_dir' that exiv2 can run over without error"""
    # we only include the below extensions because the folder has sooooo many
    # other files that we don't want to run exiv2 over.
    valid_extensions = [
        ".avif",
        ".cr3",
        ".crw",
        ".dng",
        ".eps",
        ".exv",
        ".heic",
        ".hif",
        ".jp2",
        ".jpg",
        ".jxl",
        ".pgf",
        ".png",
        ".psd",
        ".raf",
        ".tif",
        ".tiff",
        ".webp",
        ".xmp",
        ".mp4",
    ]

    excludes = [
        # all of the below files are excluded because they throw a utf-8 decoding
        # error and I did not yet have the patience to deal with that.
        "issue_1881_coverage.jpg",
        "test_issue_ghsa_mv9g_fxh2_m49m.crw",
        "issue_1827_poc.crw",
        "iptc-psAPP13s-wIPTCs-psAPP13s-wIPTCs.jpg",
        "issue_ghsa_8949_hhfh_j7rj_poc.jp2",
        "exiv2-bug444.jpg",
        "issue_1847_poc.jpg",
        "table.jpg",
        "issue_1830_poc.tiff",
        "issue_1530_poc.crw",
        "issue_ghsa_pvjp_m4f6_q984_poc.exv",
        "iptc-psAPP13s-noIPTC-psAPP13s-wIPTC.jpg",
        "issue_1530_poc.exv",
        "exiv2-bug501.jpg",
        "smiley1.jpg",
        "imagemagick.pgf",
        "iptc-psAPP13s-wIPTC-psAPP13s-noIPTC.jpg",
        "Reagan.jp2",
        "issue_ghsa_8949_hhfh_j7rj_poc.exv",
        "exiv2-bug495.jpg",
        "issue_1920_poc.tiff",
        "issue_2190_poc.jp2",
        # non-zero return code files, most of them are security POC so we don't
        # really need to worry about them here
        "issue_2340_poc.mp4",
        "issue_2345_poc.mp4",
        "issue_2366_poc.mp4",
        "issue_2376_poc.mp4",
        "issue_2377_poc.mp4",
        "2018-01-09-exiv2-crash-001.tiff",
        "cve_2017_1000126_stack-oob-read.webp",
        "exiv2-bug1247.jpg",
        "exiv2-bug841.png",
        "h02.psd",
        "issue_1097_poc.crw",
        "issue_1793_poc.heic",
        "issue_1812_poc.jp2",
        "issue_1841_poc.webp",
        "issue_428_poc1.png",
        "issue_428_poc2.png",
        "issue_428_poc3.png",
        "issue_428_poc4.png",
        "issue_428_poc5.png",
        "issue_428_poc6.png",
        "issue_428_poc7.png",
        "issue_428_poc8.png",
        "issue_712_poc.tif",
        "issue_789_poc1.png",
        "issue_790_poc2.png",
        "issue_791_poc1.webp",
        "issue_828_poc.png",
        "issue_841_poc.crw",
        "issue_843_poc.crw",
        "issue_845_poc.png",
        "issue_847_poc.pgf",
        "issue_853_poc.jpg",
        "issue_855_poc.psd",
        "issue_857_coverage.raf",
        "issue_857_poc.raf",
        "issue_869_poc.png",
        "issue_960.poc.webp",
        "issue_2160_poc.jpg",
        "issue_2178_poc.jp2",
        "issue_2268_poc.jp2",
        "issue_2270_poc.webp",
        "issue_2320_poc.jpg",
        "issue_2339_poc.tiff",
        "issue_2352_poc.jpg",
        "issue_ghsa_583f_w9pm_99r2_poc.jp2",
        "issue_ghsa_7569_phvm_vwc2_poc.jp2",
        "issue_ghsa_mxw9_qx4c_6m8v_poc.jp2",
        "pocIssue283.jpg",
        "poc_1522.jp2",
        "xmpsdk.xmp",
        # large file that creates 11Mb of output so let's exclude it
        "ReaganLargeTiff.tiff",
        # files that don't create any output
        "exiv2-empty.jpg",
        "exiv2-bug836.eps",
        "1343_empty.png",
        "issue_2027_poc.jpg",
        "issue_1815_poc.jpg",
        "exiv2.dc.dwc.i18n.jpg",
        "exiv2-bug1137.jpg",
        "issue_1833_poc.jpg",
        "exiv2-bug480.jpg",
        "issue_1934_poc4-thumb_ref.jpg",
        "heic.heic",
        "issue_1817_poc.png",
        "issue_ghsa_mvc4_g5pv_4qqq_poc.jpg",
        "issue_1881_poc.jpg",
        "issue_1934_poc1-thumb.jpg",
        "iptc-noAPP13.jpg",
        "1343_comment.png",
        "issue_ghsa_9jh3_fcc3_g6hv_poc.jpg",
        "Reagan2.jp2",
        "relax.jp2",
        # has a warning that includes absolute paths and I don't want to go
        # through the trouble of cleaning the output for a single broken file
        "issue_1845_poc.jp2",
        # fails with different error messages inside xmpsdk on win/macos than
        # on linux it's a poc file from a fuzzing test so let's just skip them
        "issue_1901_poc1.xmp",
        "issue_1901_poc2.xmp",
        "issue_1901_poc3.xmp",
        "issue_1901_poc4.xmp",
        "issue_1954_poc.xmp",
        # this test file actually contains some eixf info, but windows has
        # different output let's try and fix this later
        "exiv2-bug1044.tif",
    ]

    file_paths = [
        os.path.join(data_dir, f) for f in os.listdir(data_dir) if f not in excludes
    ]

    return [
        f
        for f in file_paths
        if os.path.isfile(os.path.join(data_dir, f))
        and os.path.splitext(f)[1] in valid_extensions
    ]


# create an empty TestCase to which we will programmatically add one test for
# each discovered file in exiv_dir/test/data/
class TestAllFiles(unittest.TestCase):
    def setUp(self):
        BT.Config.init()

    def tearDown(self):
        pass


all_files = get_valid_files(BT.Config.data_dir)


def make_test_function(filename):
    """given a filename, create a function which runs an exiv2 test against a reference file"""

    def test_func(self):
        out = BT.Output()
        out += BT.Executer("exiv2 -Pkycvt " + filename)
        BT.reportTest(os.path.basename(filename), out)

    return test_func


# for each file we create a test function with `make_test_function` and then
# assign it as a member function to our TestCase
for f in all_files:
    testname = os.path.basename(f)
    setattr(TestAllFiles, testname + "_test", make_test_function(f))

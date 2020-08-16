import os
import unittest
from system_tests import utils


class TestCases(unittest.TestCase):
    def setUp(self):
        os.chdir(utils.TEST_DIR)

    def tearDown(self):
        pass

    def test_addmoddel(self):
        jpg         = 'exiv2-empty.jpg'
        utils.copyTestFiles(jpg)
        out         = utils.runTest('addmoddel {jpg}', vars())
        out        += utils.runTest('exiv2 -pv {jpg}', vars())
        out        += ['']
        utils.reportTest('addmoddel', out)

    def test_geotag(self):
        jpg         = 'FurnaceCreekInn.jpg'
        gpx         = 'FurnaceCreekInn.gpx'
        utils.copyTestFiles(jpg, gpx)

        out         = ['--- show GPSInfo tags ---']
        out        += utils.runTest('exiv2 -pa --grep GPSInfo {jpg}', vars())

        out        += ['--- deleting the GPSInfo tags']
        for tag in utils.runTest('exiv2 -Pk --grep GPSInfo {jpg}', vars()):
            tag     = tag.rstrip(' ')
            out    += utils.runTest('exiv2 -M"del {tag}" {jpg}', vars())
        out        += utils.runTest('exiv2 -pa --grep GPS {jpg}', vars(), [0, 1])

        out        += ['--- run geotag ---']
        geotag_out  = utils.runTest('geotag -ascii -tz -8:00 {jpg} {gpx}', vars())
        geotag_out  = geotag_out[0].split(' ')[1:]
        out        += [' '.join(geotag_out)]

        out        += ['--- show GPSInfo tags ---']
        out        += utils.runTest('exiv2 -pa --grep GPSInfo {jpg}', vars())

        out        += ['']
        utils.reportTest('geotag-test', out)

    def test_io(self):
        test_files  = ['table.jpg', 'smiley2.jpg', 'ext.dat']
        utils.copyTestFiles(*test_files)
        for f in test_files:
            utils.ioTest(f)


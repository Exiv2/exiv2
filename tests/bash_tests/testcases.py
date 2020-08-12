import unittest
from system_tests import utils


class TestCases(unittest.TestCase):
    def test_geotag(self):
        testname      = 'geotag-test'
        jpg           = 'FurnaceCreekInn.jpg'
        gpx           = 'FurnaceCreekInn.gpx'
        utils.copyTestFiles(jpg, gpx)

        output        = ['--- show GPSInfo tags ---']
        output       += utils.runTest('exiv2 -pa --grep GPSInfo {jpg}', vars())

        output       += ['--- deleting the GPSInfo tags']
        for tag in utils.runTest('exiv2 -Pk --grep GPSInfo {jpg}', vars()):
            tag       = tag.rstrip(' ')
            output   += utils.runTest('exiv2 -M"del {tag}" {jpg}', vars())
        output       += utils.runTest('exiv2 -pa --grep GPS {jpg}', vars(), [0, 1])

        output       += ['--- run geotag ---']
        geotag_output = utils.runTest('geotag -ascii -tz -8:00 {jpg} {gpx}', vars())
        geotag_output = geotag_output[0].split(' ')[1:]
        output       += [' '.join(geotag_output)]

        output       += ['--- show GPSInfo tags ---']
        output       += utils.runTest('exiv2 -pa --grep GPSInfo {jpg}', vars())
        output       += ['']

        utils.reportTest(testname, output)

    def test_io(self):
        test_files = ['table.jpg', 'smiley2.jpg', 'ext.dat']
        utils.copyTestFiles(*test_files)
        for f in test_files:
            utils.ioTest(f)


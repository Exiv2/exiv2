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


    def test_conversions(self):
        jpg         = 'exiv2-empty.jpg'
        out         = []

        utils.log.info('#1 Convert Exif ImageDescription to XMP x-default langAlt value')
        out        += ['Testcase 1']
        out        += ['==========']  # 9 equal signs
        utils.copyTestFile(jpg, 'h.jpg')
        out        += utils.runTest("exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg")
        utils.rm('h.xmp')
        out        += utils.runTest('exiv2 -eX h.jpg')
        out        += utils.runTest('exiv2 -px h.xmp')
        out        += utils.runTest('exiv2 -PEkycv h.xmp')
        out        += utils.runTest('exiv2 -pi h.xmp')

        utils.log.info('#2 Convert XMP x-default langAlt value back to Exif ImageDescription')
        out        += ['']
        out        += ['Testcase 2']
        out        += ['==========']
        utils.copyTestFile(jpg, 'i.jpg')
        utils.cp('h.xmp', 'i.xmp')
        out        += utils.runTest('exiv2 -iX  i.jpg')
        out        += utils.runTest('exiv2 -px  i.jpg')
        out        += utils.runTest('exiv2 -PEkycv i.jpg')
        out        += utils.runTest('exiv2 -pi  i.jpg')

        utils.log.info('#3 Convert XMP single non-x-default langAlt value to Exif ImageDescription')
        out        += ['']
        out        += ['Testcase 3']
        out        += ['==========']
        utils.save(utils.cat('i.xmp').replace('x-default', 'de-DE'), 'j.xmp')
        utils.copyTestFile(jpg, 'j.jpg')
        out        += utils.runTest('exiv2 -iX j.jpg')
        out        += utils.runTest('exiv2 -px j.jpg')
        out        += utils.runTest('exiv2 -PEkycv j.jpg')
        out        += utils.runTest('exiv2 -pi j.jpg')

        utils.log.info("#4 This shouldn't work: No x-default, more than one language")
        out        += ['']
        out        += ['Testcase 4']
        out        += ['==========']
        utils.save(utils.cat('j.xmp').replace('<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>',
                                              '<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>'),
                   'k.xmp')
        utils.copyTestFile(jpg, 'k.jpg')
        out        += utils.runTest('exiv2 -iX k.jpg')
        out        += utils.runTest('exiv2 -px k.jpg')
        out        += utils.runTest('exiv2 -v -PEkycv k.jpg')
        out        += utils.runTest('exiv2 -v -pi k.jpg')

        utils.log.info('#5 Add a default language to the XMP file and convert to Exif and IPTC')
        out        += ['']
        out        += ['Testcase 5']
        out        += ['==========']
        utils.cp('k.xmp', 'l.xmp')
        out        += utils.runTest('''exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp''')
        out        += [utils.grep('x-default', 'l.xmp')]
        utils.copyTestFile(jpg, 'l.jpg')
        out        += utils.runTest('exiv2 -iX l.jpg')
        out        += utils.runTest('exiv2 -px -b l.jpg')
        out        += utils.runTest('exiv2 -PEkycv l.jpg')
        out        += utils.runTest('exiv2 -pi l.jpg')

        utils.log.info('#6 Convert an Exif user comment to XMP')
        out        += ['']
        out        += ['Testcase 6']
        out        += ['==========']
        utils.copyTestFile(jpg, 'm.jpg')
        out        += utils.runTest("exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg")
        out        += utils.runTest('exiv2 -PEkycv m.jpg')
        utils.rm('m.xmp')
        out        += utils.runTest('exiv2 -eX m.jpg')
        out        += utils.runTest('exiv2 -px m.xmp')
        out        += utils.runTest('exiv2 -PEkycv m.xmp')
        out        += utils.runTest('exiv2 -v -pi m.xmp')

        utils.log.info('#7 And back to Exif')
        out        += ['']
        out        += ['Testcase 7']
        out        += ['==========']
        utils.copyTestFile(jpg, 'n.jpg')
        utils.cp('m.xmp', 'n.xmp')
        out        += utils.runTest('exiv2 -iX n.jpg')
        out        += utils.runTest('exiv2 -px n.jpg')
        out        += utils.runTest('exiv2 -PEkycv n.jpg')
        out        += utils.runTest('exiv2 -v -pi n.jpg')

        utils.log.info('#8 Convert IPTC keywords to XMP')
        out        += ['']
        out        += ['Testcase 8']
        out        += ['==========']
        utils.copyTestFile(jpg, 'o.jpg')
        out        += utils.runTest('''exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg''')
        out        += utils.runTest('''exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg''')
        out        += utils.runTest('''exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg''')
        out        += utils.runTest('''exiv2 -pi o.jpg''')
        utils.rm('o.xmp')
        out        += utils.runTest('exiv2 -eX o.jpg')
        out        += utils.runTest('exiv2 -px o.xmp')
        out        += utils.runTest('exiv2 -v -PEkycv o.xmp')
        out        += utils.runTest('exiv2 -pi o.xmp')

        utils.log.info('#9 And back to IPTC')
        out        += ['']
        out        += ['Testcase 9']
        out        += ['==========']
        utils.copyTestFile(jpg, 'p.jpg')
        utils.cp('o.xmp', 'p.xmp')
        out        += utils.runTest('exiv2 -iX p.jpg')
        out        += utils.runTest('exiv2 -px p.jpg')
        out        += utils.runTest('exiv2 -v -PEkycv p.jpg')
        out        += utils.runTest('exiv2 -pi p.jpg')

        utils.log.info('#10 Convert an Exif tag to an XMP text value')
        out        += ['']
        out        += ['Testcase 10']
        out        += ['===========']  # 10 equal signs
        utils.copyTestFile(jpg, 'q.jpg')
        out        += utils.runTest("exiv2 -M'set Exif.Image.Software Exiv2' q.jpg")
        out        += utils.runTest("exiv2 -PEkycv q.jpg")
        utils.rm('q.xmp')
        out        += utils.runTest('exiv2 -eX q.jpg')
        out        += utils.runTest('exiv2 -px q.xmp')
        out        += utils.runTest('exiv2 -PEkycv q.xmp')
        out        += utils.runTest('exiv2 -v -pi q.xmp')

        utils.log.info('#11 And back to Exif')
        out        += ['']
        out        += ['Testcase 11']
        out        += ['===========']
        utils.copyTestFile(jpg, 'r.jpg')
        utils.cp('q.xmp', 'r.xmp')
        out        += utils.runTest('exiv2 -iX r.jpg')
        out        += utils.runTest('exiv2 -px r.jpg')
        out        += utils.runTest('exiv2 -PEkycv r.jpg')
        out        += utils.runTest('exiv2 -v -pi r.jpg')

        utils.log.info('#12 Convert an IPTC dataset to an XMP text value')
        out        += ['']
        out        += ['Testcase 12']
        out        += ['===========']
        utils.copyTestFile(jpg, 's.jpg')
        out        += utils.runTest("exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg")
        out        += utils.runTest("exiv2 -pi s.jpg")
        utils.rm('s.xmp')
        out        += utils.runTest('exiv2 -eX s.jpg')
        out        += utils.runTest('exiv2 -px s.xmp')
        out        += utils.runTest('exiv2 -v -PEkycv s.xmp')
        out        += utils.runTest('exiv2 -pi s.xmp')

        utils.log.info('#13 And back to IPTC')
        out        += ['']
        out        += ['Testcase 13']
        out        += ['===========']
        utils.copyTestFile(jpg, 't.jpg')
        utils.cp('s.xmp', 't.xmp')
        out        += utils.runTest('exiv2 -iX t.jpg')
        out        += utils.runTest('exiv2 -px t.jpg')
        out        += utils.runTest('exiv2 -v -PEkycv t.jpg')
        out        += utils.runTest('exiv2 -pi t.jpg')

        utils.log.info('#14 Convert a few other tags of interest from Exif/IPTC to XMP')
        out        += ['']
        out        += ['Testcase 14']
        out        += ['===========']
        utils.copyTestFile(jpg, 'u.jpg')
        out        += utils.runTest("exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.Photo.Flash 73' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg")
        out        += utils.runTest("exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg")
        out        += utils.runTest('exiv2 -PEkycv u.jpg')
        out        += utils.runTest('exiv2 -pi u.jpg')
        utils.rm('u.xmp')
        out        += utils.runTest('exiv2 -eX u.jpg')
        out        += utils.runTest('exiv2 -px u.xmp')
        out        += utils.runTest('exiv2 -PEkycv u.xmp')
        out        += utils.runTest('exiv2 -pi u.xmp')

        utils.log.info('#15 And back to Exif/IPTC')
        out        += ['']
        out        += ['Testcase 15']
        out        += ['===========']
        utils.copyTestFile(jpg, 'v.jpg')
        utils.cp('u.xmp', 'v.xmp')
        out        += utils.runTest("exiv2 -M'set Xmp.xmp.ModifyDate 2015-04-17T18:10:22Z' v.xmp")
        out        += utils.runTest('exiv2 -iX v.jpg')  # need TZ=GMT-8
        out        += utils.runTest('exiv2 -px v.jpg')
        exif_out    = utils.runTest('exiv2 -PEkycv v.jpg')
        out        += [line.replace('17 19:10:22', '18 02:10:22') for line in exif_out] # evade this test on MSVC builds (Issue #485)
        out        += utils.runTest('exiv2 -pi v.jpg')

        utils.log.info('#16 https://github.com/Exiv2/exiv2/issues/521')
        out        += ['']
        out        += ['Testcase 16']
        out        += ['===========']
        utils.copyTestFile('DSC_3079.jpg')
        out        += utils.runTest('exiv2 -px                        DSC_3079.jpg')
        out        += utils.runTest('exiv2 -M"del Xmp.mwg-rs.Regions" DSC_3079.jpg')
        out        += utils.runTest('exiv2 -px                        DSC_3079.jpg')

        out        += ['']
        utils.reportTest('conversions', out)


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


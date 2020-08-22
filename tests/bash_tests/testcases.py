import os
import unittest
from system_tests import BT


class TestCases(unittest.TestCase):


    def setUp(self):
        os.chdir(BT.Conf.tmp_dir)


    def tearDown(self):
        BT.log.buffer = ['']


    def test_addmoddel(self):
        # Test driver to run the addmoddel sample program
        jpg         = 'exiv2-empty.jpg'
        BT.copyTestFiles(jpg)
        out         = BT.excute('addmoddel {jpg}', vars())
        out        += BT.excute('exiv2 -pv {jpg}', vars())
        out        += ['']
        BT.reportTest('addmoddel', out)


    def test_conversions(self):
        # XMP parser test driver
        jpg         = 'exiv2-empty.jpg'
        out         = []

        BT.log.info('#1 Convert Exif ImageDescription to XMP x-default langAlt value')
        out        += ['Testcase 1']
        out        += ['==========']  # 9 equal signs
        BT.copyTestFile(jpg, 'h.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg")
        BT.rm('h.xmp')
        out        += BT.excute('exiv2 -eX h.jpg')
        out        += BT.excute('exiv2 -px h.xmp')
        out        += BT.excute('exiv2 -PEkycv h.xmp')
        out        += BT.excute('exiv2 -pi h.xmp')

        BT.log.info('#2 Convert XMP x-default langAlt value back to Exif ImageDescription')
        out        += ['']
        out        += ['Testcase 2']
        out        += ['==========']
        BT.copyTestFile(jpg, 'i.jpg')
        BT.cp('h.xmp', 'i.xmp')
        out        += BT.excute('exiv2 -iX  i.jpg')
        out        += BT.excute('exiv2 -px  i.jpg')
        out        += BT.excute('exiv2 -PEkycv i.jpg')
        out        += BT.excute('exiv2 -pi  i.jpg')

        BT.log.info('#3 Convert XMP single non-x-default langAlt value to Exif ImageDescription')
        out        += ['']
        out        += ['Testcase 3']
        out        += ['==========']
        BT.save(BT.cat('i.xmp').replace('x-default', 'de-DE'), 'j.xmp')
        BT.copyTestFile(jpg, 'j.jpg')
        out        += BT.excute('exiv2 -iX j.jpg')
        out        += BT.excute('exiv2 -px j.jpg')
        out        += BT.excute('exiv2 -PEkycv j.jpg')
        out        += BT.excute('exiv2 -pi j.jpg')

        BT.log.info("#4 This shouldn't work: No x-default, more than one language")
        out        += ['']
        out        += ['Testcase 4']
        out        += ['==========']
        BT.save(BT.cat('j.xmp').replace('<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>',
                                              '<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>'),
                   'k.xmp')
        BT.copyTestFile(jpg, 'k.jpg')
        out        += BT.excute('exiv2 -iX k.jpg')
        out        += BT.excute('exiv2 -px k.jpg')
        out        += BT.excute('exiv2 -v -PEkycv k.jpg')
        out        += BT.excute('exiv2 -v -pi k.jpg')

        BT.log.info('#5 Add a default language to the XMP file and convert to Exif and IPTC')
        out        += ['']
        out        += ['Testcase 5']
        out        += ['==========']
        BT.cp('k.xmp', 'l.xmp')
        out        += BT.excute('''exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp''')
        out        += [BT.grep('x-default', 'l.xmp')]
        BT.copyTestFile(jpg, 'l.jpg')
        out        += BT.excute('exiv2 -iX l.jpg')
        out        += BT.excute('exiv2 -px -b l.jpg')
        out        += BT.excute('exiv2 -PEkycv l.jpg')
        out        += BT.excute('exiv2 -pi l.jpg')

        BT.log.info('#6 Convert an Exif user comment to XMP')
        out        += ['']
        out        += ['Testcase 6']
        out        += ['==========']
        BT.copyTestFile(jpg, 'm.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg")
        out        += BT.excute('exiv2 -PEkycv m.jpg')
        BT.rm('m.xmp')
        out        += BT.excute('exiv2 -eX m.jpg')
        out        += BT.excute('exiv2 -px m.xmp')
        out        += BT.excute('exiv2 -PEkycv m.xmp')
        out        += BT.excute('exiv2 -v -pi m.xmp')

        BT.log.info('#7 And back to Exif')
        out        += ['']
        out        += ['Testcase 7']
        out        += ['==========']
        BT.copyTestFile(jpg, 'n.jpg')
        BT.cp('m.xmp', 'n.xmp')
        out        += BT.excute('exiv2 -iX n.jpg')
        out        += BT.excute('exiv2 -px n.jpg')
        out        += BT.excute('exiv2 -PEkycv n.jpg')
        out        += BT.excute('exiv2 -v -pi n.jpg')

        BT.log.info('#8 Convert IPTC keywords to XMP')
        out        += ['']
        out        += ['Testcase 8']
        out        += ['==========']
        BT.copyTestFile(jpg, 'o.jpg')
        out        += BT.excute('''exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg''')
        out        += BT.excute('''exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg''')
        out        += BT.excute('''exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg''')
        out        += BT.excute('''exiv2 -pi o.jpg''')
        BT.rm('o.xmp')
        out        += BT.excute('exiv2 -eX o.jpg')
        out        += BT.excute('exiv2 -px o.xmp')
        out        += BT.excute('exiv2 -v -PEkycv o.xmp')
        out        += BT.excute('exiv2 -pi o.xmp')

        BT.log.info('#9 And back to IPTC')
        out        += ['']
        out        += ['Testcase 9']
        out        += ['==========']
        BT.copyTestFile(jpg, 'p.jpg')
        BT.cp('o.xmp', 'p.xmp')
        out        += BT.excute('exiv2 -iX p.jpg')
        out        += BT.excute('exiv2 -px p.jpg')
        out        += BT.excute('exiv2 -v -PEkycv p.jpg')
        out        += BT.excute('exiv2 -pi p.jpg')

        BT.log.info('#10 Convert an Exif tag to an XMP text value')
        out        += ['']
        out        += ['Testcase 10']
        out        += ['===========']  # 10 equal signs
        BT.copyTestFile(jpg, 'q.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Image.Software Exiv2' q.jpg")
        out        += BT.excute("exiv2 -PEkycv q.jpg")
        BT.rm('q.xmp')
        out        += BT.excute('exiv2 -eX q.jpg')
        out        += BT.excute('exiv2 -px q.xmp')
        out        += BT.excute('exiv2 -PEkycv q.xmp')
        out        += BT.excute('exiv2 -v -pi q.xmp')

        BT.log.info('#11 And back to Exif')
        out        += ['']
        out        += ['Testcase 11']
        out        += ['===========']
        BT.copyTestFile(jpg, 'r.jpg')
        BT.cp('q.xmp', 'r.xmp')
        out        += BT.excute('exiv2 -iX r.jpg')
        out        += BT.excute('exiv2 -px r.jpg')
        out        += BT.excute('exiv2 -PEkycv r.jpg')
        out        += BT.excute('exiv2 -v -pi r.jpg')

        BT.log.info('#12 Convert an IPTC dataset to an XMP text value')
        out        += ['']
        out        += ['Testcase 12']
        out        += ['===========']
        BT.copyTestFile(jpg, 's.jpg')
        out        += BT.excute("exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg")
        out        += BT.excute("exiv2 -pi s.jpg")
        BT.rm('s.xmp')
        out        += BT.excute('exiv2 -eX s.jpg')
        out        += BT.excute('exiv2 -px s.xmp')
        out        += BT.excute('exiv2 -v -PEkycv s.xmp')
        out        += BT.excute('exiv2 -pi s.xmp')

        BT.log.info('#13 And back to IPTC')
        out        += ['']
        out        += ['Testcase 13']
        out        += ['===========']
        BT.copyTestFile(jpg, 't.jpg')
        BT.cp('s.xmp', 't.xmp')
        out        += BT.excute('exiv2 -iX t.jpg')
        out        += BT.excute('exiv2 -px t.jpg')
        out        += BT.excute('exiv2 -v -PEkycv t.jpg')
        out        += BT.excute('exiv2 -pi t.jpg')

        BT.log.info('#14 Convert a few other tags of interest from Exif/IPTC to XMP')
        out        += ['']
        out        += ['Testcase 14']
        out        += ['===========']
        BT.copyTestFile(jpg, 'u.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.Photo.Flash 73' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg")
        out        += BT.excute("exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg")
        out        += BT.excute('exiv2 -PEkycv u.jpg')
        out        += BT.excute('exiv2 -pi u.jpg')
        BT.rm('u.xmp')
        out        += BT.excute('exiv2 -eX u.jpg')
        out        += BT.excute('exiv2 -px u.xmp')
        out        += BT.excute('exiv2 -PEkycv u.xmp')
        out        += BT.excute('exiv2 -pi u.xmp')

        BT.log.info('#15 And back to Exif/IPTC')
        out        += ['']
        out        += ['Testcase 15']
        out        += ['===========']
        BT.copyTestFile(jpg, 'v.jpg')
        BT.cp('u.xmp', 'v.xmp')
        out        += BT.excute("exiv2 -M'set Xmp.xmp.ModifyDate 2015-04-17T18:10:22Z' v.xmp")
        out        += BT.excute('exiv2 -iX v.jpg')  # need TZ=GMT-8
        out        += BT.excute('exiv2 -px v.jpg')
        exif_out    = BT.excute('exiv2 -PEkycv v.jpg')
        out        += [line.replace('17 19:10:22', '18 02:10:22') for line in exif_out] # evade this test on MSVC builds (Issue #485)
        out        += BT.excute('exiv2 -pi v.jpg')

        BT.log.info('#16 https://github.com/Exiv2/exiv2/issues/521')
        out        += ['']
        out        += ['Testcase 16']
        out        += ['===========']
        BT.copyTestFile('DSC_3079.jpg')
        out        += BT.excute('exiv2 -px                        DSC_3079.jpg')
        out        += BT.excute('exiv2 -M"del Xmp.mwg-rs.Regions" DSC_3079.jpg')
        out        += BT.excute('exiv2 -px                        DSC_3079.jpg')

        out        += ['']
        BT.reportTest('conversions', out)


    def test_crw(self):
        # Test driver for CRW file operations
        crwfile     = 'exiv2-canon-powershot-s40.crw'

        BT.log.info('#1 Add and modify tags')
        cmds        = '''set Exif.Photo.ColorSpace 65535
set Exif.Canon.OwnerName Different owner
set Exif.Canon.FirmwareVersion Whatever version
set Exif.Canon.SerialNumber 1
add Exif.Canon.SerialNumber 2
set Exif.Photo.ISOSpeedRatings 155
set Exif.Photo.DateTimeOriginal 2007:11:11 09:10:11
set Exif.Image.DateTime 2020:05:26 07:31:41
set Exif.Photo.DateTimeDigitized 2020:05:26 07:31:42'''
        cmdfile     = 'cmdfile1'
        BT.save(cmds, cmdfile)
        BT.copyTestFile(crwfile)
        out         = BT.excute('exiv2 -v -pt           {crwfile}', vars())
        out        += BT.excute('exiv2 -v -m{cmdfile}   {crwfile}', vars())
        out        += BT.excute('exiv2 -v -pt           {crwfile}', vars())

        BT.log.info('#2 Delete tags')
        BT.copyTestFile(crwfile)
        out        += BT.excute("exiv2 -v -pt           {crwfile}", vars())
        out        += BT.excute("exiv2 -v -M'del Exif.Canon.OwnerName'    {crwfile}", vars())
        out        += BT.excute("exiv2 -v -pt           {crwfile}", vars())

        # sed evades TZ issue on MSVC builds #1221
        out         = [line.replace('23 19:54', '23 18:54') for line in out]
        out         = [line.replace('24 01:54', '23 18:54') for line in out]

        out        += ['']
        BT.reportTest('crw-test', out)


    def test_exifdata(self):
        # Test driver for exifdata copy construction and assignment unit tests
        out         = []
        for jpg in ['exiv2-gc.jpg', 'exiv2-canon-powershot-s40.jpg', 'exiv2-nikon-d70.jpg']:
            BT.copyTestFile(jpg)
            out    += BT.excute('exifdata-test {jpg}', vars())
        out        += ['']
        BT.reportTest('exifdata-test', out)


    def test_icc(self):
        # Test driver for exiv2.exe ICC support (-pS, -pC, -eC, -iC)

        def test1120(filename):
            # --comment and -dc clobbered by writing ICC/JPG
            if filename == 'Reagan2.jp2':
                return []
            if filename == 'exiv2-bug1199.webp':
                out  = BT.excute('exiv2 --comment abcdefg   {filename}', vars(), [0,1])
                out += BT.excute('exiv2 -pS                 {filename}', vars())
                out += ['']
            else:
                out  = BT.excute('exiv2 --comment abcdefg   {filename}', vars())
                out += BT.excute('exiv2 -pS                 {filename}', vars())
            out += BT.excute('exiv2 -pc                 {filename}', vars())
            out += BT.excute('exiv2 -dc                 {filename}', vars())
            out += BT.excute('exiv2 -pS                 {filename}', vars())
            return out or []

        # num = 1074  # ICC Profile Support
        out = []
        for filename in ['Reagan.jpg',
                         'exiv2-bug1199.webp',
                         'ReaganLargePng.png',
                         'ReaganLargeJpg.jpg',
                         'Reagan2.jp2']:  # 1272 ReaganLargeTiff.tiff
            stub         = filename.split('.')[0]
            iccname      = stub + '.icc'

            BT.copyTestFiles('large.icc', 'small.icc', filename)
            out         += BT.excute('exiv2 -pS          {filename}', vars())
            icc_bytes    = BT.excute('exiv2 -pC          {filename}', vars(), return_bytes=True)
            BT.save(icc_bytes, stub + '_1.icc')
            out         += BT.excute('exiv2 -eC --force  {filename}', vars())
            BT.mv(iccname, stub + '_2.icc')
            out         += test1120(filename)

            BT.copyTestFile('large.icc', iccname)
            out         += BT.excute('exiv2 -iC          {filename}', vars())
            icc_bytes    = BT.excute('exiv2 -pC          {filename}', vars(), return_bytes=True)
            BT.save(icc_bytes, stub + '_large_1.icc')
            out         += BT.excute('exiv2 -pS          {filename}', vars())
            out         += BT.excute('exiv2 -eC --force  {filename}', vars())
            BT.mv(iccname, stub + '_large_2.icc')
            out         += test1120(filename)

            BT.copyTestFile('small.icc', iccname)
            out         += BT.excute('exiv2 -iC          {filename}', vars())
            icc_bytes    = BT.excute('exiv2 -pC          {filename}', vars(), return_bytes=True)
            BT.save(icc_bytes, stub + '_small_1.icc')
            out         += BT.excute('exiv2 -pS          {filename}', vars())
            out         += BT.excute('exiv2 -eC --force  {filename}', vars())
            BT.mv(iccname, stub + '_small_2.icc')
            out         += test1120(filename)

            for f in [stub, stub + '_small', stub + '_large']:
                for i in [1, 2]:
                    out += [BT.md5sum('{}_{}.icc'.format(f, i))]

        out += ['']
        BT.reportTest('icc-test', out)


    def test_geotag(self):
        # Test driver for geotag
        jpg         = 'FurnaceCreekInn.jpg'
        gpx         = 'FurnaceCreekInn.gpx'
        BT.copyTestFiles(jpg, gpx)

        out         = ['--- show GPSInfo tags ---']
        out        += BT.excute('exiv2 -pa --grep GPSInfo {jpg}', vars())

        out        += ['--- deleting the GPSInfo tags']
        for tag in BT.excute('exiv2 -Pk --grep GPSInfo {jpg}', vars()):
            tag     = tag.rstrip(' ')
            out    += BT.excute('exiv2 -M"del {tag}" {jpg}', vars())
        out        += BT.excute('exiv2 -pa --grep GPS {jpg}', vars(), [0, 1])

        out        += ['--- run geotag ---']
        geotag_out  = BT.excute('geotag -ascii -tz -8:00 {jpg} {gpx}', vars())
        geotag_out  = geotag_out[0].split(' ')[1:]
        out        += [' '.join(geotag_out)]

        out        += ['--- show GPSInfo tags ---']
        out        += BT.excute('exiv2 -pa --grep GPSInfo {jpg}', vars())

        out        += ['']
        BT.reportTest('geotag-test', out)


    def test_io(self):
        # Test driver for file i/o
        test_files  = ['table.jpg', 'smiley2.jpg', 'ext.dat']
        BT.copyTestFiles(*test_files)
        for f in test_files:
            BT.ioTest(f)

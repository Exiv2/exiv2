import os
import re
import unittest

from system_tests import BT


class TestCases(unittest.TestCase):


    def setUp(self):
        BT.Conf.init()


    def tearDown(self):
        pass


    def test_addmoddel(self):
        # Test driver to run the addmoddel sample program
        jpg         = 'exiv2-empty.jpg'
        BT.copyTestFile(jpg)
        out         = BT.Output()
        out        += BT.excute('addmoddel {jpg}', vars())
        out        += BT.excute('exiv2 -pv {jpg}', vars())
        BT.reportTest('addmoddel', out)


    def test_conversions(self):
        # XMP parser test driver
        jpg         = 'exiv2-empty.jpg'
        out         = BT.Output()

        BT.log.info('#1 Convert Exif ImageDescription to XMP x-default langAlt value')
        out        += 'Testcase 1'
        out        += '=========='  # 9 equal signs
        BT.copyTestFile(jpg, 'h.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg")
        BT.rm('h.xmp')
        out        += BT.excute('exiv2 -eX h.jpg')
        out        += BT.excute('exiv2 -px h.xmp')
        out        += BT.excute('exiv2 -PEkycv h.xmp')
        out        += BT.excute('exiv2 -pi h.xmp')

        BT.log.info('#2 Convert XMP x-default langAlt value back to Exif ImageDescription')
        out        += ''
        out        += 'Testcase 2'
        out        += '=========='
        BT.copyTestFile(jpg, 'i.jpg')
        BT.cp('h.xmp', 'i.xmp')
        out        += BT.excute('exiv2 -iX  i.jpg')
        out        += BT.excute('exiv2 -px  i.jpg')
        out        += BT.excute('exiv2 -PEkycv i.jpg')
        out        += BT.excute('exiv2 -pi  i.jpg')

        BT.log.info('#3 Convert XMP single non-x-default langAlt value to Exif ImageDescription')
        out        += ''
        out        += 'Testcase 3'
        out        += '=========='
        BT.save(BT.cat('i.xmp').replace('x-default', 'de-DE'), 'j.xmp')
        BT.copyTestFile(jpg, 'j.jpg')
        out        += BT.excute('exiv2 -iX j.jpg')
        out        += BT.excute('exiv2 -px j.jpg')
        out        += BT.excute('exiv2 -PEkycv j.jpg')
        out        += BT.excute('exiv2 -pi j.jpg')

        BT.log.info("#4 This shouldn't work: No x-default, more than one language")
        out        += ''
        out        += 'Testcase 4'
        out        += '=========='
        BT.save(BT.cat('j.xmp').replace('<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>',
                                              '<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>'),
                   'k.xmp')
        BT.copyTestFile(jpg, 'k.jpg')
        out        += BT.excute('exiv2 -iX k.jpg')
        out        += BT.excute('exiv2 -px k.jpg')
        out        += BT.excute('exiv2 -v -PEkycv k.jpg')
        out        += BT.excute('exiv2 -v -pi k.jpg')

        BT.log.info('#5 Add a default language to the XMP file and convert to Exif and IPTC')
        out        += ''
        out        += 'Testcase 5'
        out        += '=========='
        BT.cp('k.xmp', 'l.xmp')
        out        += BT.excute('''exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp''')
        out        += BT.grep('x-default', 'l.xmp')
        BT.copyTestFile(jpg, 'l.jpg')
        out        += BT.excute('exiv2 -iX l.jpg')
        out        += BT.excute('exiv2 -px -b l.jpg')
        out        += BT.excute('exiv2 -PEkycv l.jpg')
        out        += BT.excute('exiv2 -pi l.jpg')

        BT.log.info('#6 Convert an Exif user comment to XMP')
        out        += ''
        out        += 'Testcase 6'
        out        += '=========='
        BT.copyTestFile(jpg, 'm.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg")
        out        += BT.excute('exiv2 -PEkycv m.jpg')
        BT.rm('m.xmp')
        out        += BT.excute('exiv2 -eX m.jpg')
        out        += BT.excute('exiv2 -px m.xmp')
        out        += BT.excute('exiv2 -PEkycv m.xmp')
        out        += BT.excute('exiv2 -v -pi m.xmp')

        BT.log.info('#7 And back to Exif')
        out        += ''
        out        += 'Testcase 7'
        out        += '=========='
        BT.copyTestFile(jpg, 'n.jpg')
        BT.cp('m.xmp', 'n.xmp')
        out        += BT.excute('exiv2 -iX n.jpg')
        out        += BT.excute('exiv2 -px n.jpg')
        out        += BT.excute('exiv2 -PEkycv n.jpg')
        out        += BT.excute('exiv2 -v -pi n.jpg')

        BT.log.info('#8 Convert IPTC keywords to XMP')
        out        += ''
        out        += 'Testcase 8'
        out        += '=========='
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
        out        += ''
        out        += 'Testcase 9'
        out        += '=========='
        BT.copyTestFile(jpg, 'p.jpg')
        BT.cp('o.xmp', 'p.xmp')
        out        += BT.excute('exiv2 -iX p.jpg')
        out        += BT.excute('exiv2 -px p.jpg')
        out        += BT.excute('exiv2 -v -PEkycv p.jpg')
        out        += BT.excute('exiv2 -pi p.jpg')

        BT.log.info('#10 Convert an Exif tag to an XMP text value')
        out        += ''
        out        += 'Testcase 10'
        out        += '==========='  # 10 equal signs
        BT.copyTestFile(jpg, 'q.jpg')
        out        += BT.excute("exiv2 -M'set Exif.Image.Software Exiv2' q.jpg")
        out        += BT.excute("exiv2 -PEkycv q.jpg")
        BT.rm('q.xmp')
        out        += BT.excute('exiv2 -eX q.jpg')
        out        += BT.excute('exiv2 -px q.xmp')
        out        += BT.excute('exiv2 -PEkycv q.xmp')
        out        += BT.excute('exiv2 -v -pi q.xmp')

        BT.log.info('#11 And back to Exif')
        out        += ''
        out        += 'Testcase 11'
        out        += '==========='
        BT.copyTestFile(jpg, 'r.jpg')
        BT.cp('q.xmp', 'r.xmp')
        out        += BT.excute('exiv2 -iX r.jpg')
        out        += BT.excute('exiv2 -px r.jpg')
        out        += BT.excute('exiv2 -PEkycv r.jpg')
        out        += BT.excute('exiv2 -v -pi r.jpg')

        BT.log.info('#12 Convert an IPTC dataset to an XMP text value')
        out        += ''
        out        += 'Testcase 12'
        out        += '==========='
        BT.copyTestFile(jpg, 's.jpg')
        out        += BT.excute("exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg")
        out        += BT.excute("exiv2 -pi s.jpg")
        BT.rm('s.xmp')
        out        += BT.excute('exiv2 -eX s.jpg')
        out        += BT.excute('exiv2 -px s.xmp')
        out        += BT.excute('exiv2 -v -PEkycv s.xmp')
        out        += BT.excute('exiv2 -pi s.xmp')

        BT.log.info('#13 And back to IPTC')
        out        += ''
        out        += 'Testcase 13'
        out        += '==========='
        BT.copyTestFile(jpg, 't.jpg')
        BT.cp('s.xmp', 't.xmp')
        out        += BT.excute('exiv2 -iX t.jpg')
        out        += BT.excute('exiv2 -px t.jpg')
        out        += BT.excute('exiv2 -v -PEkycv t.jpg')
        out        += BT.excute('exiv2 -pi t.jpg')

        BT.log.info('#14 Convert a few other tags of interest from Exif/IPTC to XMP')
        out        += ''
        out        += 'Testcase 14'
        out        += '==========='
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
        out        += ''
        out        += 'Testcase 15'
        out        += '==========='
        BT.copyTestFile(jpg, 'v.jpg')
        BT.cp('u.xmp', 'v.xmp')
        out        += BT.excute("exiv2 -M'set Xmp.xmp.ModifyDate 2015-04-17T18:10:22Z' v.xmp")
        out        += BT.excute('exiv2 -iX v.jpg')  # need TZ=GMT-8
        out        += BT.excute('exiv2 -px v.jpg')
        out        += BT.excute('exiv2 -PEkycv v.jpg').replace('17 19:10:22', '18 02:10:22') # evade this test on MSVC builds (Issue #485)
        out        += BT.excute('exiv2 -pi v.jpg')

        BT.log.info('#16 https://github.com/Exiv2/exiv2/issues/521')
        out        += ''
        out        += 'Testcase 16'
        out        += '==========='
        BT.copyTestFile('DSC_3079.jpg')
        out        += BT.excute('exiv2 -px                        DSC_3079.jpg')
        out        += BT.excute('exiv2 -M"del Xmp.mwg-rs.Regions" DSC_3079.jpg')
        out        += BT.excute('exiv2 -px                        DSC_3079.jpg')

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
        out         = BT.Output()
        out        += BT.excute('exiv2 -v -pt           {crwfile}', vars())
        out        += BT.excute('exiv2 -v -m{cmdfile}   {crwfile}', vars())
        out        += BT.excute('exiv2 -v -pt           {crwfile}', vars())

        BT.log.info('#2 Delete tags')
        BT.copyTestFile(crwfile)
        out        += BT.excute("exiv2 -v -pt           {crwfile}", vars())
        out        += BT.excute("exiv2 -v -M'del Exif.Canon.OwnerName'    {crwfile}", vars())
        out        += BT.excute("exiv2 -v -pt           {crwfile}", vars())

        # sed evades TZ issue on MSVC builds #1221
        out         = str(out).replace('23 19:54', '23 18:54').replace('24 01:54', '23 18:54')

        BT.reportTest('crw-test', out)


    def test_exifdata(self):
        # Test driver for exifdata copy construction and assignment unit tests
        out         = BT.Output()
        for jpg in ['exiv2-gc.jpg', 'exiv2-canon-powershot-s40.jpg', 'exiv2-nikon-d70.jpg']:
            BT.copyTestFile(jpg)
            out    += BT.excute('exifdata-test {jpg}', vars())
        BT.reportTest('exifdata-test', out)


    def test_exiv2(self):
        # Add each image to the following three lists.
        # The image basename in the second and third lists
        # is the Exif timestamp adjusted by -12:01:01.
        images_1 = [
            'exiv2-empty.jpg',
            'exiv2-canon-powershot-s40.jpg',
            'exiv2-nikon-e990.jpg',
            'exiv2-nikon-d70.jpg',
            'exiv2-nikon-e950.jpg',
            'exiv2-canon-eos-300d.jpg',
            'exiv2-kodak-dc210.jpg',
            'exiv2-fujifilm-finepix-s2pro.jpg',
            'exiv2-sigma-d10.jpg',
            'exiv2-olympus-c8080wz.jpg',
            'exiv2-panasonic-dmc-fz5.jpg',
            'exiv2-sony-dsc-w7.jpg',
            'exiv2-canon-eos-20d.jpg',
            'exiv2-canon-eos-d30.jpg',
            'exiv2-canon-powershot-a520.jpg',]

        images_2 = [
            'exiv2-empty.jpg',
            '20031214_000043.jpg',
            '20000506_020544.jpg',
            '20040329_224245.jpg',
            '20010405_235039.jpg',
            '20030925_201850.jpg',
            '20001026_044550.jpg',
            '20030926_111535.jpg',
            '20040316_075137.jpg',
            '20040208_093744.jpg',
            '20050218_212016.jpg',
            '20050527_051833.jpg',
            '20060802_095200.jpg',
            '20001004_015404.jpg',
            '20060127_225027.jpg',]

        images_3 = [
            'exiv2-empty.exv',
            '20031214_000043.exv',
            '20000506_020544.exv',
            '20040329_224245.exv',
            '20010405_235039.exv',
            '20030925_201850.exv',
            '20001026_044550.exv',
            '20030926_111535.exv',
            '20040316_075137.exv',
            '20040208_093744.exv',
            '20050218_212016.exv',
            '20050527_051833.exv',
            '20060802_095200.exv',
            '20001004_015404.exv',
            '20060127_225027.exv',]

        images_1_str = ' '.join(images_1)
        images_2_str = ' '.join(images_2)
        images_3_str = ' '.join(images_3)

        for i in images_1:
            BT.copyTestFile(i)

        out  = BT.Output()
        out += 'Exiv2 test directory -----------------------------------------------------'
        out += 'tmp/'
        out += ''

        out += 'Exiv2 version ------------------------------------------------------------'
        # Tweak this to avoid a maintenance headache with test/data/exiv2-test.out
        out += re.sub(r'exiv2.*', 'exiv2 0.27.0.0 (__ bit build)', BT.excute('exiv2 -u -V'))
        out += ''

        out += 'Exiv2 help ---------------------------------------------------------------'
        out += BT.excute('exiv2 -u -h')
        out += ''
        out += ''

        out += 'Adjust -------------------------------------------------------------------'
        out += BT.excute('exiv2 -u -v -a-12:01:01 adjust {images_1_str}', vars(), expected_returncodes=[253])
        out += ''

        out += 'Rename -------------------------------------------------------------------'
        out += BT.excute('exiv2 -u -vf rename {images_1_str}', vars(), expected_returncodes=[253])
        out += ''

        out += 'Print --------------------------------------------------------------------'
        out += BT.excute('exiv2 -u -v print {images_2_str}', vars(), expected_returncodes=[253])
        out += ''
        out += BT.excute('exiv2 -u -v -b -pt print {images_2_str}', vars())
        stdout, stderr = BT.excute('exiv2 -u -v -b -pt print {images_2_str}', vars(), mix_stdout_and_stderr=False)
        BT.save(stdout, 'iii')
        out += stderr
        out += ''

        out += 'Extract Exif data --------------------------------------------------------'
        out += BT.excute('exiv2 -u -vf extract {images_2_str}', vars())
        out += ''

        out += 'Extract Thumbnail --------------------------------------------------------'
        out += BT.excute('exiv2 -u -vf -et extract {images_2_str}', vars(), expected_returncodes=[253])
        stdout, stderr = BT.excute('exiv2 -u -v -b -pt print {images_3_str}', vars(), mix_stdout_and_stderr=False)
        BT.save(stdout, 'jjj')
        out += stderr
        out += ''

        out += 'Compare image data and extracted data ------------------------------------'
        out += BT.diff('iii', 'jjj')
        out += ''

        out += 'Delete Thumbnail ---------------------------------------------------------'
        out += BT.excute('exiv2 -u -v -dt delete {images_2_str}', vars())
        out += BT.excute('exiv2 -u -vf -et extract {images_2_str}', vars(), expected_returncodes=[253])
        out += ''

        out += 'Delete Exif data ---------------------------------------------------------'
        out += BT.excute('exiv2 -u -v delete {images_2_str}', vars())
        out += BT.excute('exiv2 -u -v print {images_2_str}', vars(), expected_returncodes=[253])
        out += ''

        out += 'Insert Exif data ---------------------------------------------------------'
        out += BT.excute('exiv2 -u -v insert {images_2_str}', vars())
        stdout, stderr = BT.excute('exiv2 -u -v -b -pt print {images_3_str}', vars(), mix_stdout_and_stderr=False)
        BT.save(stdout, 'kkk')
        out += stderr
        out += ''

        out += 'Compare original and inserted image data ---------------------------------'
        out += BT.diff('iii', 'kkk')

        BT.reportTest('exiv2-test', out)


    def test_geotag(self):
        # Test driver for geotag
        jpg         = 'FurnaceCreekInn.jpg'
        gpx         = 'FurnaceCreekInn.gpx'
        for i in [jpg, gpx]:
            BT.copyTestFile(i)

        out         = BT.Output()
        out        += '--- show GPSInfo tags ---'
        out        += BT.excute('exiv2 -pa --grep GPSInfo {jpg}', vars())

        out        += '--- deleting the GPSInfo tags'
        for tag in BT.excute('exiv2 -Pk --grep GPSInfo {jpg}', vars()).split('\n'):
            tag = tag.rstrip(' ')
            out    += BT.excute('exiv2 -M"del {tag}" {jpg}', vars())
        out        += BT.excute('exiv2 -pa --grep GPS {jpg}', vars(), expected_returncodes=[0, 1])

        out        += '--- run geotag ---'
        geotag_out  = BT.excute('geotag -ascii -tz -8:00 {jpg} {gpx}', vars())
        geotag_out  = geotag_out.split('\n')[0].split(' ')[1:]
        out        += ' '.join(geotag_out)

        out        += '--- show GPSInfo tags ---'
        out        += BT.excute('exiv2 -pa --grep GPSInfo {jpg}', vars())

        BT.reportTest('geotag-test', out)


    def test_icc(self):
        # Test driver for exiv2.exe ICC support (-pS, -pC, -eC, -iC)

        def test1120(img):
            # --comment and -dc clobbered by writing ICC/JPG
            out      = BT.Output()
            if img  == 'Reagan2.jp2':
                return
            if img  == 'exiv2-bug1199.webp':
                out += BT.excute('exiv2 --comment abcdefg   {img}', vars(), expected_returncodes=[0,1])
                out += BT.excute('exiv2 -pS                 {img}', vars())
                out += ''
            else:
                out += BT.excute('exiv2 --comment abcdefg   {img}', vars())
                out += BT.excute('exiv2 -pS                 {img}', vars())
            out     += BT.excute('exiv2 -pc                 {img}', vars())
            out     += BT.excute('exiv2 -dc                 {img}', vars())
            out     += BT.excute('exiv2 -pS                 {img}', vars())
            return str(out) or None

        # num = 1074  # ICC Profile Support
        out = BT.Output()
        for img in ['Reagan.jpg',
                    'exiv2-bug1199.webp',
                    'ReaganLargePng.png',
                    'ReaganLargeJpg.jpg',
                    'Reagan2.jp2']:  # 1272 ReaganLargeTiff.tiff
            stub         = img.split('.')[0]
            iccname      = stub + '.icc'

            for i in ['large.icc', 'small.icc', img]:
                BT.copyTestFile(i)

            out         += BT.excute('exiv2 -pS          {img}', vars())
            BT.save(BT.excute('exiv2 -pC                 {img}', vars(), return_in_bytes=True),
                    stub + '_1.icc')
            out         += BT.excute('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_2.icc')
            out         += test1120(img)

            BT.copyTestFile('large.icc', iccname)
            out         += BT.excute('exiv2 -iC          {img}', vars())
            BT.save(BT.excute('exiv2 -pC                 {img}', vars(), return_in_bytes=True),
                    stub + '_large_1.icc')
            out         += BT.excute('exiv2 -pS          {img}', vars())
            out         += BT.excute('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_large_2.icc')
            out         += test1120(img)

            BT.copyTestFile('small.icc', iccname)
            out         += BT.excute('exiv2 -iC          {img}', vars())
            BT.save(BT.excute('exiv2 -pC                 {img}', vars(), return_in_bytes=True),
                    stub + '_small_1.icc')
            out         += BT.excute('exiv2 -pS          {img}', vars())
            out         += BT.excute('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_small_2.icc')
            out         += test1120(img)

            for f in [stub, stub + '_small', stub + '_large']:
                for i in [1, 2]:
                    out += BT.md5sum('{}_{}.icc'.format(f, i))

        BT.reportTest('icc-test', out)


    def test_io(self):
        # Test driver for file i/o
        test_files  = ['table.jpg', 'smiley2.jpg', 'ext.dat']
        for f in test_files:
            BT.copyTestFile(f)
            BT.ioTest(f)

        # Test http I/O
        def sniff(*files):
            result       = [str(os.path.getsize(i)) for i in files]
            result      += [BT.md5sum(i) for i in files]
            return ' '.join(result)

        server_bind      = '127.0.0.1'
        server_port      = 12760
        server_url       = 'http://{}:{}'.format(server_bind, server_port)
        server           = BT.HttpServer(bind=server_bind, port=server_port, work_dir=os.path.join(BT.Conf.data_dir))
        try:
            server.start()
            out          = BT.Output()
            for img in ['table.jpg', 'Reagan.tiff', 'exiv2-bug922a.jpg']:
                files    = ['s0', 's1', 's2', '{}/{}'.format(server_url, img)]
                out     += BT.excute('iotest ' + ' '.join(files))
                for f in files:
                    out += BT.excute('exiv2 -g City -g DateTime {f}', vars())


            for num in ['0', '10', '1000']:
                out     += BT.excute('iotest s0 s1 s2 {server_url}/table.jpg {num}', vars())
                out     += sniff('s0', 's1', 's2', os.path.join(BT.Conf.data_dir, 'table.jpg'))
        finally:
            server.stop()
            # print('keep the server running...')

        BT.reportTest('iotest', out)

import os
import re
import unittest

from system_tests import BT


class TestCases(unittest.TestCase):


    def setUp(self):
        BT.Config.init()


    def tearDown(self):
        pass


    def addmoddel_test(self):
        # Test driver to run the addmoddel sample program
        jpg         = 'exiv2-empty.jpg'
        BT.copyTestFile(jpg)
        out         = BT.Output()
        out        += BT.execute('addmoddel {jpg}', vars())
        out        += BT.execute('exiv2 -pv {jpg}', vars())
        BT.reportTest('addmoddel', out)


    def conversions_test(self):
        # XMP parser test driver
        jpg         = 'exiv2-empty.jpg'
        out         = BT.Output()

        BT.log.info('#1 Convert Exif ImageDescription to XMP x-default langAlt value')
        out        += 'Testcase 1'
        out        += '=========='  # 9 equal signs
        BT.copyTestFile(jpg, 'h.jpg')
        out        += BT.execute("exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg")
        BT.rm('h.xmp')
        out        += BT.execute('exiv2 -eX h.jpg')
        out        += BT.execute('exiv2 -px h.xmp')
        out        += BT.execute('exiv2 -PEkycv h.xmp')
        out        += BT.execute('exiv2 -pi h.xmp')

        BT.log.info('#2 Convert XMP x-default langAlt value back to Exif ImageDescription')
        out        += ''
        out        += 'Testcase 2'
        out        += '=========='
        BT.copyTestFile(jpg, 'i.jpg')
        BT.cp('h.xmp', 'i.xmp')
        out        += BT.execute('exiv2 -iX  i.jpg')
        out        += BT.execute('exiv2 -px  i.jpg')
        out        += BT.execute('exiv2 -PEkycv i.jpg')
        out        += BT.execute('exiv2 -pi  i.jpg')

        BT.log.info('#3 Convert XMP single non-x-default langAlt value to Exif ImageDescription')
        out        += ''
        out        += 'Testcase 3'
        out        += '=========='
        BT.save(BT.cat('i.xmp').replace('x-default', 'de-DE'), 'j.xmp')
        BT.copyTestFile(jpg, 'j.jpg')
        out        += BT.execute('exiv2 -iX j.jpg')
        out        += BT.execute('exiv2 -px j.jpg')
        out        += BT.execute('exiv2 -PEkycv j.jpg')
        out        += BT.execute('exiv2 -pi j.jpg')

        BT.log.info("#4 This shouldn't work: No x-default, more than one language")
        out        += ''
        out        += 'Testcase 4'
        out        += '=========='
        BT.save(BT.cat('j.xmp').replace('<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>',
                                              '<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>'),
                   'k.xmp')
        BT.copyTestFile(jpg, 'k.jpg')
        out        += BT.execute('exiv2 -iX k.jpg')
        out        += BT.execute('exiv2 -px k.jpg')
        out        += BT.execute('exiv2 -v -PEkycv k.jpg')
        out        += BT.execute('exiv2 -v -pi k.jpg')

        BT.log.info('#5 Add a default language to the XMP file and convert to Exif and IPTC')
        out        += ''
        out        += 'Testcase 5'
        out        += '=========='
        BT.cp('k.xmp', 'l.xmp')
        out        += BT.execute('''exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp''')
        out        += BT.grep('x-default', 'l.xmp')
        BT.copyTestFile(jpg, 'l.jpg')
        out        += BT.execute('exiv2 -iX l.jpg')
        out        += BT.execute('exiv2 -px -b l.jpg')
        out        += BT.execute('exiv2 -PEkycv l.jpg')
        out        += BT.execute('exiv2 -pi l.jpg')

        BT.log.info('#6 Convert an Exif user comment to XMP')
        out        += ''
        out        += 'Testcase 6'
        out        += '=========='
        BT.copyTestFile(jpg, 'm.jpg')
        out        += BT.execute("exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg")
        out        += BT.execute('exiv2 -PEkycv m.jpg')
        BT.rm('m.xmp')
        out        += BT.execute('exiv2 -eX m.jpg')
        out        += BT.execute('exiv2 -px m.xmp')
        out        += BT.execute('exiv2 -PEkycv m.xmp')
        out        += BT.execute('exiv2 -v -pi m.xmp')

        BT.log.info('#7 And back to Exif')
        out        += ''
        out        += 'Testcase 7'
        out        += '=========='
        BT.copyTestFile(jpg, 'n.jpg')
        BT.cp('m.xmp', 'n.xmp')
        out        += BT.execute('exiv2 -iX n.jpg')
        out        += BT.execute('exiv2 -px n.jpg')
        out        += BT.execute('exiv2 -PEkycv n.jpg')
        out        += BT.execute('exiv2 -v -pi n.jpg')

        BT.log.info('#8 Convert IPTC keywords to XMP')
        out        += ''
        out        += 'Testcase 8'
        out        += '=========='
        BT.copyTestFile(jpg, 'o.jpg')
        out        += BT.execute('''exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg''')
        out        += BT.execute('''exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg''')
        out        += BT.execute('''exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg''')
        out        += BT.execute('''exiv2 -pi o.jpg''')
        BT.rm('o.xmp')
        out        += BT.execute('exiv2 -eX o.jpg')
        out        += BT.execute('exiv2 -px o.xmp')
        out        += BT.execute('exiv2 -v -PEkycv o.xmp')
        out        += BT.execute('exiv2 -pi o.xmp')

        BT.log.info('#9 And back to IPTC')
        out        += ''
        out        += 'Testcase 9'
        out        += '=========='
        BT.copyTestFile(jpg, 'p.jpg')
        BT.cp('o.xmp', 'p.xmp')
        out        += BT.execute('exiv2 -iX p.jpg')
        out        += BT.execute('exiv2 -px p.jpg')
        out        += BT.execute('exiv2 -v -PEkycv p.jpg')
        out        += BT.execute('exiv2 -pi p.jpg')

        BT.log.info('#10 Convert an Exif tag to an XMP text value')
        out        += ''
        out        += 'Testcase 10'
        out        += '==========='  # 10 equal signs
        BT.copyTestFile(jpg, 'q.jpg')
        out        += BT.execute("exiv2 -M'set Exif.Image.Software Exiv2' q.jpg")
        out        += BT.execute("exiv2 -PEkycv q.jpg")
        BT.rm('q.xmp')
        out        += BT.execute('exiv2 -eX q.jpg')
        out        += BT.execute('exiv2 -px q.xmp')
        out        += BT.execute('exiv2 -PEkycv q.xmp')
        out        += BT.execute('exiv2 -v -pi q.xmp')

        BT.log.info('#11 And back to Exif')
        out        += ''
        out        += 'Testcase 11'
        out        += '==========='
        BT.copyTestFile(jpg, 'r.jpg')
        BT.cp('q.xmp', 'r.xmp')
        out        += BT.execute('exiv2 -iX r.jpg')
        out        += BT.execute('exiv2 -px r.jpg')
        out        += BT.execute('exiv2 -PEkycv r.jpg')
        out        += BT.execute('exiv2 -v -pi r.jpg')

        BT.log.info('#12 Convert an IPTC dataset to an XMP text value')
        out        += ''
        out        += 'Testcase 12'
        out        += '==========='
        BT.copyTestFile(jpg, 's.jpg')
        out        += BT.execute("exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg")
        out        += BT.execute("exiv2 -pi s.jpg")
        BT.rm('s.xmp')
        out        += BT.execute('exiv2 -eX s.jpg')
        out        += BT.execute('exiv2 -px s.xmp')
        out        += BT.execute('exiv2 -v -PEkycv s.xmp')
        out        += BT.execute('exiv2 -pi s.xmp')

        BT.log.info('#13 And back to IPTC')
        out        += ''
        out        += 'Testcase 13'
        out        += '==========='
        BT.copyTestFile(jpg, 't.jpg')
        BT.cp('s.xmp', 't.xmp')
        out        += BT.execute('exiv2 -iX t.jpg')
        out        += BT.execute('exiv2 -px t.jpg')
        out        += BT.execute('exiv2 -v -PEkycv t.jpg')
        out        += BT.execute('exiv2 -pi t.jpg')

        BT.log.info('#14 Convert a few other tags of interest from Exif/IPTC to XMP')
        out        += ''
        out        += 'Testcase 14'
        out        += '==========='
        BT.copyTestFile(jpg, 'u.jpg')
        out        += BT.execute("exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.Photo.Flash 73' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg")
        out        += BT.execute("exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg")
        out        += BT.execute('exiv2 -PEkycv u.jpg')
        out        += BT.execute('exiv2 -pi u.jpg')
        BT.rm('u.xmp')
        out        += BT.execute('exiv2 -eX u.jpg')
        out        += BT.execute('exiv2 -px u.xmp')
        out        += BT.execute('exiv2 -PEkycv u.xmp')
        out        += BT.execute('exiv2 -pi u.xmp')

        BT.log.info('#15 And back to Exif/IPTC')
        out        += ''
        out        += 'Testcase 15'
        out        += '==========='
        BT.copyTestFile(jpg, 'v.jpg')
        BT.cp('u.xmp', 'v.xmp')
        out        += BT.execute("exiv2 -M'set Xmp.xmp.ModifyDate 2015-04-17T18:10:22Z' v.xmp")
        out        += BT.execute('exiv2 -iX v.jpg')  # need TZ=GMT-8
        out        += BT.execute('exiv2 -px v.jpg')
        out        += BT.execute('exiv2 -PEkycv v.jpg').replace('17 19:10:22', '18 02:10:22') # evade this test on MSVC builds (Issue #485)
        out        += BT.execute('exiv2 -pi v.jpg')

        BT.log.info('#16 https://github.com/Exiv2/exiv2/issues/521')
        out        += ''
        out        += 'Testcase 16'
        out        += '==========='
        BT.copyTestFile('DSC_3079.jpg')
        out        += BT.execute('exiv2 -px                        DSC_3079.jpg')
        out        += BT.execute('exiv2 -M"del Xmp.mwg-rs.Regions" DSC_3079.jpg')
        out        += BT.execute('exiv2 -px                        DSC_3079.jpg')

        BT.reportTest('conversions', out)


    def crw_test(self):
        # Test driver for CRW file operations
        crwfile     = 'exiv2-canon-powershot-s40.crw'

        BT.log.info('#1 Add and modify tags')
        cmds        = '''
set Exif.Photo.ColorSpace 65535
set Exif.Canon.OwnerName Different owner
set Exif.Canon.FirmwareVersion Whatever version
set Exif.Canon.SerialNumber 1
add Exif.Canon.SerialNumber 2
set Exif.Photo.ISOSpeedRatings 155
set Exif.Photo.DateTimeOriginal 2007:11:11 09:10:11
set Exif.Image.DateTime 2020:05:26 07:31:41
set Exif.Photo.DateTimeDigitized 2020:05:26 07:31:42
'''.lstrip('\n')
        cmdfile     = 'cmdfile1'
        BT.save(cmds, cmdfile)
        BT.copyTestFile(crwfile)
        out         = BT.Output()
        out        += BT.execute('exiv2 -v -pt           {crwfile}', vars())
        out        += BT.execute('exiv2 -v -m{cmdfile}   {crwfile}', vars())
        out        += BT.execute('exiv2 -v -pt           {crwfile}', vars())

        BT.log.info('#2 Delete tags')
        BT.copyTestFile(crwfile)
        out        += BT.execute("exiv2 -v -pt           {crwfile}", vars())
        out        += BT.execute("exiv2 -v -M'del Exif.Canon.OwnerName'    {crwfile}", vars())
        out        += BT.execute("exiv2 -v -pt           {crwfile}", vars())

        # sed evades TZ issue on MSVC builds #1221
        out         = str(out).replace('23 19:54', '23 18:54').replace('24 01:54', '23 18:54')

        BT.reportTest('crw-test', out)


    def exifdata_test(self):
        # Test driver for exifdata copy construction and assignment unit tests
        out         = BT.Output()
        for jpg in ['exiv2-gc.jpg', 'exiv2-canon-powershot-s40.jpg', 'exiv2-nikon-d70.jpg']:
            BT.copyTestFile(jpg)
            out    += BT.execute('exifdata-test {jpg}', vars())
        BT.reportTest('exifdata-test', out)


    def exiv2_test(self):
        # Add each image to the following three lists.
        # The image basename in the second and third lists
        # is the Exif timestamp adjusted by -12:01:01.
        images_1 = [
            'exiv2-empty.jpg'
            ,'exiv2-canon-powershot-s40.jpg'
            ,'exiv2-nikon-e990.jpg'
            ,'exiv2-nikon-d70.jpg'
            ,'exiv2-nikon-e950.jpg'
            ,'exiv2-canon-eos-300d.jpg'
            ,'exiv2-kodak-dc210.jpg'
            ,'exiv2-fujifilm-finepix-s2pro.jpg'
            ,'exiv2-sigma-d10.jpg'
            ,'exiv2-olympus-c8080wz.jpg'
            ,'exiv2-panasonic-dmc-fz5.jpg'
            ,'exiv2-sony-dsc-w7.jpg'
            ,'exiv2-canon-eos-20d.jpg'
            ,'exiv2-canon-eos-d30.jpg'
            ,'exiv2-canon-powershot-a520.jpg']

        images_2 = [
            'exiv2-empty.jpg'
            ,'20031214_000043.jpg'
            ,'20000506_020544.jpg'
            ,'20040329_224245.jpg'
            ,'20010405_235039.jpg'
            ,'20030925_201850.jpg'
            ,'20001026_044550.jpg'
            ,'20030926_111535.jpg'
            ,'20040316_075137.jpg'
            ,'20040208_093744.jpg'
            ,'20050218_212016.jpg'
            ,'20050527_051833.jpg'
            ,'20060802_095200.jpg'
            ,'20001004_015404.jpg'
            ,'20060127_225027.jpg']

        images_3 = [
            'exiv2-empty.exv'
            ,'20031214_000043.exv'
            ,'20000506_020544.exv'
            ,'20040329_224245.exv'
            ,'20010405_235039.exv'
            ,'20030925_201850.exv'
            ,'20001026_044550.exv'
            ,'20030926_111535.exv'
            ,'20040316_075137.exv'
            ,'20040208_093744.exv'
            ,'20050218_212016.exv'
            ,'20050527_051833.exv'
            ,'20060802_095200.exv'
            ,'20001004_015404.exv'
            ,'20060127_225027.exv']

        images_1_str = ' '.join(images_1)
        images_2_str = ' '.join(images_2)
        images_3_str = ' '.join(images_3)

        for i in images_1:
            BT.copyTestFile(i)

        out  = BT.Output()
        out += 'Exiv2 test directory -----------------------------------------------------'
        out += 'tmp/'

        out += '\nExiv2 version ------------------------------------------------------------'
        # Tweak this to avoid a maintenance headache with test/data/exiv2-test.out
        out += re.sub(r'exiv2.*', 'exiv2 0.27.0.0 (__ bit build)', BT.execute('exiv2 -u -V'))

        out += '\nExiv2 help ---------------------------------------------------------------'
        out += BT.execute('exiv2 -u -h')

        out += '\n\nAdjust -------------------------------------------------------------------'
        out += BT.execute('exiv2 -u -v -a-12:01:01 adjust {images_1_str}', vars(), expected_returncodes=[253])

        out += '\nRename -------------------------------------------------------------------'
        out += BT.execute('exiv2 -u -vf rename {images_1_str}', vars(), expected_returncodes=[253])

        out += '\nPrint --------------------------------------------------------------------'
        out += BT.execute('exiv2 -u -v print {images_2_str}', vars(), expected_returncodes=[253])
        out += ''
        out += BT.execute('exiv2 -u -v -b -pt print {images_2_str}', vars())
        stdout, stderr = BT.execute('exiv2 -u -v -b -pt print {images_2_str}', vars(), mix_stdout_and_stderr=False)
        BT.save(stdout, 'iii')
        out += stderr

        out += '\nExtract Exif data --------------------------------------------------------'
        out += BT.execute('exiv2 -u -vf extract {images_2_str}', vars())

        out += '\nExtract Thumbnail --------------------------------------------------------'
        out += BT.execute('exiv2 -u -vf -et extract {images_2_str}', vars(), expected_returncodes=[253])
        stdout, stderr = BT.execute('exiv2 -u -v -b -pt print {images_3_str}', vars(), mix_stdout_and_stderr=False)
        BT.save(stdout, 'jjj')
        out += stderr

        out += '\nCompare image data and extracted data ------------------------------------'
        out += BT.diff('iii', 'jjj')

        out += '\nDelete Thumbnail ---------------------------------------------------------'
        out += BT.execute('exiv2 -u -v -dt delete {images_2_str}', vars())
        out += BT.execute('exiv2 -u -vf -et extract {images_2_str}', vars(), expected_returncodes=[253])

        out += '\nDelete Exif data ---------------------------------------------------------'
        out += BT.execute('exiv2 -u -v delete {images_2_str}', vars())
        out += BT.execute('exiv2 -u -v print {images_2_str}', vars(), expected_returncodes=[253])

        out += '\nInsert Exif data ---------------------------------------------------------'
        out += BT.execute('exiv2 -u -v insert {images_2_str}', vars())
        stdout, stderr = BT.execute('exiv2 -u -v -b -pt print {images_3_str}', vars(), mix_stdout_and_stderr=False)
        BT.save(stdout, 'kkk')
        out += stderr

        out += '\nCompare original and inserted image data ---------------------------------'
        out += BT.diff('iii', 'kkk')

        BT.reportTest('exiv2-test', out)


    def geotag_test(self):
        # Test driver for geotag
        jpg         = 'FurnaceCreekInn.jpg'
        gpx         = 'FurnaceCreekInn.gpx'
        for i in [jpg, gpx]:
            BT.copyTestFile(i)

        out         = BT.Output()
        out        += '--- show GPSInfo tags ---'
        out        += BT.execute('exiv2 -pa --grep GPSInfo {jpg}', vars())

        out        += '--- deleting the GPSInfo tags'
        for tag in BT.execute('exiv2 -Pk --grep GPSInfo {jpg}', vars()).split('\n'):
            tag = tag.rstrip(' ')
            out    += BT.execute('exiv2 -M"del {tag}" {jpg}', vars())
        out        += BT.execute('exiv2 -pa --grep GPS {jpg}', vars(), expected_returncodes=[0, 1])

        out        += '--- run geotag ---'
        geotag_out  = BT.execute('geotag -ascii -tz -8:00 {jpg} {gpx}', vars())
        geotag_out  = geotag_out.split('\n')[0].split(' ')[1:]
        out        += ' '.join(geotag_out)

        out        += '--- show GPSInfo tags ---'
        out        += BT.execute('exiv2 -pa --grep GPSInfo {jpg}', vars())

        BT.reportTest('geotag-test', out)


    def icc_test(self):
        # Test driver for exiv2.exe ICC support (-pS, -pC, -eC, -iC)

        def test1120(img):
            # --comment and -dc clobbered by writing ICC/JPG
            out      = BT.Output()
            if img  == 'Reagan2.jp2':
                return
            if img  == 'exiv2-bug1199.webp':
                out += BT.execute('exiv2 --comment abcdefg   {img}', vars(), expected_returncodes=[0,1])
                out += BT.execute('exiv2 -pS                 {img}', vars())
                out += ''
            else:
                out += BT.execute('exiv2 --comment abcdefg   {img}', vars())
                out += BT.execute('exiv2 -pS                 {img}', vars())
            out     += BT.execute('exiv2 -pc                 {img}', vars())
            out     += BT.execute('exiv2 -dc                 {img}', vars())
            out     += BT.execute('exiv2 -pS                 {img}', vars())
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

            out         += BT.execute('exiv2 -pS          {img}', vars())
            BT.save(BT.execute('exiv2 -pC                 {img}', vars(), return_bytes=True),
                    stub + '_1.icc')
            out         += BT.execute('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_2.icc')
            out         += test1120(img)

            BT.copyTestFile('large.icc', iccname)
            out         += BT.execute('exiv2 -iC          {img}', vars())
            BT.save(BT.execute('exiv2 -pC                 {img}', vars(), return_bytes=True),
                    stub + '_large_1.icc')
            out         += BT.execute('exiv2 -pS          {img}', vars())
            out         += BT.execute('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_large_2.icc')
            out         += test1120(img)

            BT.copyTestFile('small.icc', iccname)
            out         += BT.execute('exiv2 -iC          {img}', vars())
            BT.save(BT.execute('exiv2 -pC                 {img}', vars(), return_bytes=True),
                    stub + '_small_1.icc')
            out         += BT.execute('exiv2 -pS          {img}', vars())
            out         += BT.execute('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_small_2.icc')
            out         += test1120(img)

            for f in [stub, stub + '_small', stub + '_large']:
                for i in [1, 2]:
                    out += BT.md5sum('{}_{}.icc'.format(f, i))

        BT.reportTest('icc-test', out)


    def image_test(self):
        test_files = [
            'table.jpg'
            ,'smiley1.jpg'
            ,'smiley2.jpg']
        erase_test_files = [
            'glider.exv'
            ,'iptc-noAPP13.jpg'
            ,'iptc-psAPP13-noIPTC.jpg'
            ,'iptc-psAPP13-noIPTC-psAPP13-wIPTC.jpg'
            ,'iptc-psAPP13s-noIPTC-psAPP13s-wIPTC.jpg'
            ,'iptc-psAPP13s-wIPTC-psAPP13s-noIPTC.jpg'
            ,'iptc-psAPP13s-wIPTCs-psAPP13s-wIPTCs.jpg'
            ,'iptc-psAPP13-wIPTC1-psAPP13-wIPTC2.jpg'
            ,'iptc-psAPP13-wIPTCbeg.jpg'
            ,'iptc-psAPP13-wIPTCempty.jpg'
            ,'iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg'
            ,'iptc-psAPP13-wIPTCend.jpg'
            ,'iptc-psAPP13-wIPTCmid1-wIPTCempty-wIPTCmid2.jpg'
            ,'iptc-psAPP13-wIPTCmid.jpg'
            ,'iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg']

        pass_count  = 0
        fail_count  = 0
        out = BT.Output()

        out += '\n--- Erase all tests ---'
        for i in test_files + erase_test_files:
            if BT.eraseTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out        += 'Failed: ' + i

        out += '\n--- Copy all tests ---'
        for num, src in enumerate(test_files, 1):
            for dst in test_files:
                if BT.copyTest(num, src, dst):
                    pass_count += 1
                else:
                    fail_count += 1
                    out        += 'Failed: {}'.format((num, src, dst))

        out += '\n--- Copy iptc tests ---'
        for num, src in enumerate(test_files, 1):
            for dst in test_files:
                if BT.iptcTest(num, src, dst):
                    pass_count += 1
                else:
                    fail_count += 1
                    out        += 'Failed: {}'.format((num, src, dst))

        out += '\n--------------------\n'
        out += '{} passed, {} failed\n'.format(pass_count, fail_count)
        if fail_count:
            raise RuntimeError(str(out) + '\n' + BT.log.to_str())


    def io_test(self):
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

        server_url = '{}:{}'.format(BT.Config.exiv2_http,
                                    BT.Config.exiv2_port)
        server = BT.HttpServer(bind=BT.Config.exiv2_http.lstrip('http://'),
                               port=BT.Config.exiv2_port,
                               work_dir=BT.Config.data_dir)
        try:
            server.start()
            out          = BT.Output()
            for img in ['table.jpg', 'Reagan.tiff', 'exiv2-bug922a.jpg']:
                files    = ['s0', 's1', 's2', '{}/{}'.format(server_url, img)]
                out     += BT.execute('iotest ' + ' '.join(files))
                for f in files:
                    out += BT.execute('exiv2 -g City -g DateTime {f}', vars())


            for num in ['0', '10', '1000']:
                out     += BT.execute('iotest s0 s1 s2 {server_url}/table.jpg {num}', vars())
                out     += sniff('s0', 's1', 's2', os.path.join(BT.Config.data_dir, 'table.jpg'))

        except Exception as e:
            BT.log.error(e)
            raise RuntimeError('\n' + BT.log.to_str())

        finally:
            server.stop()   # While you're debugging, you can comment this line to keep the server running
            pass

        BT.reportTest('iotest', out)


    def iptc_test(self):
        # Test driver for Iptc metadata
        test_files = [
            'glider.exv'
            ,'iptc-noAPP13.jpg'
            ,'iptc-psAPP13-noIPTC.jpg'
            ,'iptc-psAPP13-noIPTC-psAPP13-wIPTC.jpg'
            ,'iptc-psAPP13s-noIPTC-psAPP13s-wIPTC.jpg'
            ,'iptc-psAPP13s-wIPTC-psAPP13s-noIPTC.jpg'
            ,'iptc-psAPP13s-wIPTCs-psAPP13s-wIPTCs.jpg'
            ,'iptc-psAPP13-wIPTC1-psAPP13-wIPTC2.jpg'
            ,'iptc-psAPP13-wIPTCbeg.jpg'
            ,'iptc-psAPP13-wIPTCempty.jpg'
            ,'iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg'
            ,'iptc-psAPP13-wIPTCend.jpg'
            ,'iptc-psAPP13-wIPTCmid1-wIPTCempty-wIPTCmid2.jpg'
            ,'iptc-psAPP13-wIPTCmid.jpg'
            ,'iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg'
            ,'smiley1.jpg'
            ,'smiley2.jpg'
            ,'table.jpg']

        pass_count  = 0
        fail_count  = 0
        out = BT.Output()

        out += '\n--- Read tests ---'
        for i in test_files:
            if BT.printTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out += 'Failed: ' + i

        out += '\n--- Remove tests ---'
        for i in test_files:
            if BT.removeTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out += 'Failed: ' + i

        out += '\n--- Add/Mod tests ---'
        for i in test_files:
            if BT.addModTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out += 'Failed: ' + i

        out += '\n--- Extended tests ---'
        for i in test_files:
            if BT.extendedTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out += 'Failed: ' + i

        out += '\n--------------------\n'
        out += '{} passed, {} failed\n'.format(pass_count, fail_count)
        if fail_count:
            raise RuntimeError(str(out) + '\n' + BT.log.to_str())


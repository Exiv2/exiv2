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
        jpg      = 'exiv2-empty.jpg'
        BT.copyTestFile(jpg)
        out      = BT.Output()
        out     += BT.Executer('addmoddel {jpg}', vars())
        out     += BT.Executer('exiv2 -pv {jpg}', vars())
        BT.reportTest('addmoddel', out)


    def conversions_test(self):
        # XMP parser test driver
        jpg      = 'exiv2-empty.jpg'
        out      = BT.Output()

        BT.log.info('#1 Convert Exif ImageDescription to XMP x-default langAlt value')
        out     += 'Testcase 1'
        out     += '=========='  # 9 equal signs
        BT.copyTestFile(jpg, 'h.jpg')
        out     += BT.Executer("exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg")
        BT.rm('h.xmp')
        out     += BT.Executer('exiv2 -eX h.jpg')
        out     += BT.Executer('exiv2 -px h.xmp')
        out     += BT.Executer('exiv2 -PEkycv h.xmp')
        out     += BT.Executer('exiv2 -pi h.xmp')

        BT.log.info('#2 Convert XMP x-default langAlt value back to Exif ImageDescription')
        out     += ''
        out     += 'Testcase 2'
        out     += '=========='
        BT.copyTestFile(jpg, 'i.jpg')
        BT.cp('h.xmp', 'i.xmp')
        out     += BT.Executer('exiv2 -iX  i.jpg')
        out     += BT.Executer('exiv2 -px  i.jpg')
        out     += BT.Executer('exiv2 -PEkycv i.jpg')
        out     += BT.Executer('exiv2 -pi  i.jpg')

        BT.log.info('#3 Convert XMP single non-x-default langAlt value to Exif ImageDescription')
        out     += ''
        out     += 'Testcase 3'
        out     += '=========='
        BT.save(BT.cat('i.xmp').replace('x-default', 'de-DE'), 'j.xmp')
        BT.copyTestFile(jpg, 'j.jpg')
        out     += BT.Executer('exiv2 -iX j.jpg')
        out     += BT.Executer('exiv2 -px j.jpg')
        out     += BT.Executer('exiv2 -PEkycv j.jpg')
        out     += BT.Executer('exiv2 -pi j.jpg')

        BT.log.info("#4 This shouldn't work: No x-default, more than one language")
        out     += ''
        out     += 'Testcase 4'
        out     += '=========='
        BT.save(BT.cat('j.xmp').replace('<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>',
                                        '<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>')
                ,'k.xmp')
        BT.copyTestFile(jpg, 'k.jpg')
        out     += BT.Executer('exiv2 -iX k.jpg')
        out     += BT.Executer('exiv2 -px k.jpg')
        out     += BT.Executer('exiv2 -v -PEkycv k.jpg')
        out     += BT.Executer('exiv2 -v -pi k.jpg')

        BT.log.info('#5 Add a default language to the XMP file and convert to Exif and IPTC')
        out     += ''
        out     += 'Testcase 5'
        out     += '=========='
        BT.cp('k.xmp', 'l.xmp')
        out     += BT.Executer('''exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp''')
        out     += BT.grep('x-default', 'l.xmp')
        BT.copyTestFile(jpg, 'l.jpg')
        out     += BT.Executer('exiv2 -iX l.jpg')
        out     += BT.Executer('exiv2 -px -b l.jpg')
        out     += BT.Executer('exiv2 -PEkycv l.jpg')
        out     += BT.Executer('exiv2 -pi l.jpg')

        BT.log.info('#6 Convert an Exif user comment to XMP')
        out     += ''
        out     += 'Testcase 6'
        out     += '=========='
        BT.copyTestFile(jpg, 'm.jpg')
        out     += BT.Executer("exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg")
        out     += BT.Executer('exiv2 -PEkycv m.jpg')
        BT.rm('m.xmp')
        out     += BT.Executer('exiv2 -eX m.jpg')
        out     += BT.Executer('exiv2 -px m.xmp')
        out     += BT.Executer('exiv2 -PEkycv m.xmp')
        out     += BT.Executer('exiv2 -v -pi m.xmp')

        BT.log.info('#7 And back to Exif')
        out     += ''
        out     += 'Testcase 7'
        out     += '=========='
        BT.copyTestFile(jpg, 'n.jpg')
        BT.cp('m.xmp', 'n.xmp')
        out     += BT.Executer('exiv2 -iX n.jpg')
        out     += BT.Executer('exiv2 -px n.jpg')
        out     += BT.Executer('exiv2 -PEkycv n.jpg')
        out     += BT.Executer('exiv2 -v -pi n.jpg')

        BT.log.info('#8 Convert IPTC keywords to XMP')
        out     += ''
        out     += 'Testcase 8'
        out     += '=========='
        BT.copyTestFile(jpg, 'o.jpg')
        out     += BT.Executer('''exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg''')
        out     += BT.Executer('''exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg''')
        out     += BT.Executer('''exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg''')
        out     += BT.Executer('''exiv2 -pi o.jpg''')
        BT.rm('o.xmp')
        out     += BT.Executer('exiv2 -eX o.jpg')
        out     += BT.Executer('exiv2 -px o.xmp')
        out     += BT.Executer('exiv2 -v -PEkycv o.xmp')
        out     += BT.Executer('exiv2 -pi o.xmp')

        BT.log.info('#9 And back to IPTC')
        out     += ''
        out     += 'Testcase 9'
        out     += '=========='
        BT.copyTestFile(jpg, 'p.jpg')
        BT.cp('o.xmp', 'p.xmp')
        out     += BT.Executer('exiv2 -iX p.jpg')
        out     += BT.Executer('exiv2 -px p.jpg')
        out     += BT.Executer('exiv2 -v -PEkycv p.jpg')
        out     += BT.Executer('exiv2 -pi p.jpg')

        BT.log.info('#10 Convert an Exif tag to an XMP text value')
        out     += ''
        out     += 'Testcase 10'
        out     += '==========='  # 10 equal signs
        BT.copyTestFile(jpg, 'q.jpg')
        out     += BT.Executer("exiv2 -M'set Exif.Image.Software Exiv2' q.jpg")
        out     += BT.Executer("exiv2 -PEkycv q.jpg")
        BT.rm('q.xmp')
        out     += BT.Executer('exiv2 -eX q.jpg')
        out     += BT.Executer('exiv2 -px q.xmp')
        out     += BT.Executer('exiv2 -PEkycv q.xmp')
        out     += BT.Executer('exiv2 -v -pi q.xmp')

        BT.log.info('#11 And back to Exif')
        out     += ''
        out     += 'Testcase 11'
        out     += '==========='
        BT.copyTestFile(jpg, 'r.jpg')
        BT.cp('q.xmp', 'r.xmp')
        out     += BT.Executer('exiv2 -iX r.jpg')
        out     += BT.Executer('exiv2 -px r.jpg')
        out     += BT.Executer('exiv2 -PEkycv r.jpg')
        out     += BT.Executer('exiv2 -v -pi r.jpg')

        BT.log.info('#12 Convert an IPTC dataset to an XMP text value')
        out     += ''
        out     += 'Testcase 12'
        out     += '==========='
        BT.copyTestFile(jpg, 's.jpg')
        out     += BT.Executer("exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg")
        out     += BT.Executer("exiv2 -pi s.jpg")
        BT.rm('s.xmp')
        out     += BT.Executer('exiv2 -eX s.jpg')
        out     += BT.Executer('exiv2 -px s.xmp')
        out     += BT.Executer('exiv2 -v -PEkycv s.xmp')
        out     += BT.Executer('exiv2 -pi s.xmp')

        BT.log.info('#13 And back to IPTC')
        out     += ''
        out     += 'Testcase 13'
        out     += '==========='
        BT.copyTestFile(jpg, 't.jpg')
        BT.cp('s.xmp', 't.xmp')
        out     += BT.Executer('exiv2 -iX t.jpg')
        out     += BT.Executer('exiv2 -px t.jpg')
        out     += BT.Executer('exiv2 -v -PEkycv t.jpg')
        out     += BT.Executer('exiv2 -pi t.jpg')

        BT.log.info('#14 Convert a few other tags of interest from Exif/IPTC to XMP')
        out     += ''
        out     += 'Testcase 14'
        out     += '==========='
        BT.copyTestFile(jpg, 'u.jpg')
        out     += BT.Executer("exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.Photo.Flash 73' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg")
        out     += BT.Executer("exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg")
        out     += BT.Executer('exiv2 -PEkycv u.jpg')
        out     += BT.Executer('exiv2 -pi u.jpg')
        BT.rm('u.xmp')
        out     += BT.Executer('exiv2 -eX u.jpg')
        out     += BT.Executer('exiv2 -px u.xmp')
        out     += BT.Executer('exiv2 -PEkycv u.xmp')
        out     += BT.Executer('exiv2 -pi u.xmp')

        BT.log.info('#15 And back to Exif/IPTC')
        out     += ''
        out     += 'Testcase 15'
        out     += '==========='
        BT.copyTestFile(jpg, 'v.jpg')
        BT.cp('u.xmp', 'v.xmp')
        out     += BT.Executer("exiv2 -M'set Xmp.xmp.ModifyDate 2015-04-17T18:10:22Z' v.xmp")
        out     += BT.Executer('exiv2 -iX v.jpg')  # need TZ=GMT-8
        out     += BT.Executer('exiv2 -px v.jpg')
        out     += BT.Executer('exiv2 -PEkycv v.jpg').stdout.replace('17 19:10:22', '18 02:10:22') # evade this test on MSVC builds (Issue #485)
        out     += BT.Executer('exiv2 -pi v.jpg')

        BT.log.info('#16 https://github.com/Exiv2/exiv2/issues/521')
        out     += ''
        out     += 'Testcase 16'
        out     += '==========='
        BT.copyTestFile('DSC_3079.jpg')
        out     += BT.Executer('exiv2 -px                        DSC_3079.jpg')
        out     += BT.Executer('exiv2 -M"del Xmp.mwg-rs.Regions" DSC_3079.jpg')
        out     += BT.Executer('exiv2 -px                        DSC_3079.jpg')

        # Ignore output differences on Windows
        for pair in [
            ('charset="Jis"'    , 'charset=Jis'),
            ('charset="Unicode"', 'charset=Unicode'),
            (' 9  Rocknroll'    , "11  Rock'n'roll"),
            ('Rocknroll'        , "Rock'n'roll")
        ]:
            out  = str(out).replace(pair[0], pair[1])

        BT.reportTest('conversions', out)


    def crw_test(self):
        # Test driver for CRW file operations
        crwfile  = 'exiv2-canon-powershot-s40.crw'

        BT.log.info('#1 Add and modify tags')
        cmds     = '''
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
        cmdfile  = 'cmdfile1'
        BT.save(cmds, cmdfile)
        BT.copyTestFile(crwfile)
        out      = BT.Output()
        out     += BT.Executer('exiv2 -v -pt           {crwfile}', vars())
        out     += BT.Executer('exiv2 -v -m{cmdfile}   {crwfile}', vars())
        out     += BT.Executer('exiv2 -v -pt           {crwfile}', vars())

        BT.log.info('#2 Delete tags')
        BT.copyTestFile(crwfile)
        out     += BT.Executer("exiv2 -v -pt           {crwfile}", vars())
        out     += BT.Executer("exiv2 -v -M'del Exif.Canon.OwnerName'    {crwfile}", vars())
        out     += BT.Executer("exiv2 -v -pt           {crwfile}", vars())

        # sed evades TZ issue on MSVC builds #1221
        out      = str(out).replace('23 19:54', '23 18:54').replace('24 01:54', '23 18:54')

        BT.reportTest('crw-test', out)


    def exifdata_test(self):
        # Test driver for exifdata copy construction and assignment unit tests
        out      = BT.Output()
        for jpg in ['exiv2-gc.jpg', 'exiv2-canon-powershot-s40.jpg', 'exiv2-nikon-d70.jpg']:
            BT.copyTestFile(jpg)
            out += BT.Executer('exifdata-test {jpg}', vars())
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
            ,'exiv2-canon-powershot-a520.jpg'
        ]

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
            ,'20060127_225027.jpg'
        ]

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
            ,'20060127_225027.exv'
        ]

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
        out += re.sub(r'exiv2.*', 'exiv2 0.27.0.0 (__ bit build)', BT.Executer('exiv2 -u -V').stdout)

        out += '\nExiv2 help ---------------------------------------------------------------'
        out += BT.Executer('exiv2 -u -h')

        out += '\n\nAdjust -------------------------------------------------------------------'
        out += BT.Executer('exiv2 -u -v -a-12:01:01 adjust  {images_1_str}', vars(), assert_returncode=[253])

        out += '\nRename -------------------------------------------------------------------'
        out += BT.Executer('exiv2 -u -vf rename             {images_1_str}', vars(), assert_returncode=[253])

        out += '\nPrint --------------------------------------------------------------------'
        out += BT.Executer('exiv2 -u -v print               {images_2_str}', vars(), assert_returncode=[253])
        out += ''
        out += BT.Executer('exiv2 -u -v -b -pt print        {images_2_str}', vars())
        e    = BT.Executer('exiv2 -u -v -b -pt print        {images_2_str}', vars(), redirect_stderr_to_stdout=False, decode_output=False)
        BT.save(e.stdout, 'iii')
        out += e.stderr.decode()

        out += '\nExtract Exif data --------------------------------------------------------'
        out += BT.Executer('exiv2 -u -vf extract            {images_2_str}', vars())

        out += '\nExtract Thumbnail --------------------------------------------------------'
        out += BT.Executer('exiv2 -u -vf -et extract        {images_2_str}', vars(), assert_returncode=[253])
        e    = BT.Executer('exiv2 -u -v -b -pt print        {images_3_str}', vars(), redirect_stderr_to_stdout=False, decode_output=False)
        BT.save(e.stdout, 'jjj')
        out += e.stderr.decode()

        out += '\nCompare image data and extracted data ------------------------------------'
        out += BT.diff('iii', 'jjj')

        out += '\nDelete Thumbnail ---------------------------------------------------------'
        out += BT.Executer('exiv2 -u -v -dt delete          {images_2_str}', vars())
        out += BT.Executer('exiv2 -u -vf -et extract        {images_2_str}', vars(), assert_returncode=[253])

        out += '\nDelete Exif data ---------------------------------------------------------'
        out += BT.Executer('exiv2 -u -v delete              {images_2_str}', vars())
        out += BT.Executer('exiv2 -u -v print               {images_2_str}', vars(), assert_returncode=[253])

        out += '\nInsert Exif data ---------------------------------------------------------'
        out += BT.Executer('exiv2 -u -v insert              {images_2_str}', vars())
        e    = BT.Executer('exiv2 -u -v -b -pt print        {images_3_str}', vars(), redirect_stderr_to_stdout=False, decode_output=False)
        BT.save(e.stdout, 'kkk')
        out += e.stderr.decode()

        out += '\nCompare original and inserted image data ---------------------------------'
        out += BT.diff('iii', 'kkk')

        BT.reportTest('exiv2-test', out)


    def geotag_test(self):
        # Test driver for geotag
        jpg      = 'FurnaceCreekInn.jpg'
        gpx      = 'FurnaceCreekInn.gpx'
        for i in [jpg, gpx]:
            BT.copyTestFile(i)

        out      = BT.Output()
        out     += '--- show GPSInfo tags ---'
        out     += BT.Executer('exiv2 -pa --grep GPSInfo    {jpg}', vars())

        out     += '--- deleting the GPSInfo tags'
        for tag in BT.Executer('exiv2 -Pk --grep GPSInfo    {jpg}', vars()).stdout.split('\n'):
            tag  = tag.rstrip(' ')
            out += BT.Executer('exiv2 -M"del {tag}"         {jpg}', vars())
        out     += BT.Executer('exiv2 -pa --grep GPS        {jpg}', vars(), assert_returncode=[0, 1])

        out     += '--- run geotag ---'
        e        = BT.Executer('geotag -ascii -tz -8:00     {jpg} {gpx}', vars())
        out     += ' '.join(e.stdout.split('\n')[0].split(' ')[1:])

        out     += '--- show GPSInfo tags ---'
        out     += BT.Executer('exiv2 -pa --grep GPSInfo    {jpg}', vars())

        BT.reportTest('geotag-test', out)


    def icc_test(self):
        # Test driver for exiv2.exe ICC support (-pS, -pC, -eC, -iC)

        def test1120(img):
            # --comment and -dc clobbered by writing ICC/JPG
            out      = BT.Output()
            if img  == 'Reagan2.jp2':
                return
            if img  == 'exiv2-bug1199.webp':
                out += BT.Executer('exiv2 --comment abcdefg   {img}', vars(), assert_returncode=[0, 1])
                out += BT.Executer('exiv2 -pS                 {img}', vars())
                out += ''
            else:
                out += BT.Executer('exiv2 --comment abcdefg   {img}', vars())
                out += BT.Executer('exiv2 -pS                 {img}', vars())
            out     += BT.Executer('exiv2 -pc                 {img}', vars())
            out     += BT.Executer('exiv2 -dc                 {img}', vars())
            out     += BT.Executer('exiv2 -pS                 {img}', vars())
            return str(out) or None

        # num = 1074  # ICC Profile Support
        out = BT.Output()
        for img in ['Reagan.jpg'
                    ,'exiv2-bug1199.webp'
                    ,'ReaganLargePng.png'
                    ,'ReaganLargeJpg.jpg'
                    ,'Reagan2.jp2'      # 1272 ReaganLargeTiff.tiff
                    ]:
            stub      = img.split('.')[0]
            iccname   = stub + '.icc'

            for i in ['large.icc', 'small.icc', img]:
                BT.copyTestFile(i)

            out      += BT.Executer('exiv2 -pS          {img}', vars())
            e         = BT.Executer('exiv2 -pC          {img}', vars(), compatible_output=False, decode_output=False)
            BT.save(e.stdout, stub + '_1.icc')
            out      += BT.Executer('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_2.icc')
            out      += test1120(img)

            BT.copyTestFile('large.icc', iccname)
            out      += BT.Executer('exiv2 -iC          {img}', vars())
            e         = BT.Executer('exiv2 -pC          {img}', vars(), compatible_output=False, decode_output=False)
            BT.save(e.stdout, stub + '_large_1.icc')
            out      += BT.Executer('exiv2 -pS          {img}', vars())
            out      += BT.Executer('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_large_2.icc')
            out      += test1120(img)

            BT.copyTestFile('small.icc', iccname)
            out      += BT.Executer('exiv2 -iC          {img}', vars())
            e         = BT.Executer('exiv2 -pC          {img}', vars(), compatible_output=False, decode_output=False)
            BT.save(e.stdout, stub + '_small_1.icc')
            out      += BT.Executer('exiv2 -pS          {img}', vars())
            out      += BT.Executer('exiv2 -eC --force  {img}', vars())
            BT.mv(iccname, stub + '_small_2.icc')
            out      += test1120(img)

            for f in [stub, stub + '_small', stub + '_large']:
                for i in [1, 2]:
                    out += BT.md5sum('{}_{}.icc'.format(f, i))

        BT.reportTest('icc-test', out)


    def image_test(self):
        test_files = ['table.jpg', 'smiley1.jpg', 'smiley2.jpg']
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
            ,'iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg'
        ]

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
        test_files = ['table.jpg', 'smiley2.jpg', 'ext.dat']
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
        server     = BT.HttpServer(bind=BT.Config.exiv2_http.lstrip('http://'),
                                   port=BT.Config.exiv2_port,
                                   work_dir=BT.Config.data_dir)
        try:
            server.start()
            out          = BT.Output()
            for img in ['table.jpg', 'Reagan.tiff', 'exiv2-bug922a.jpg']:
                files    = ['s0', 's1', 's2', '{}/{}'.format(server_url, img)]
                out     += BT.Executer('iotest ' + ' '.join(files))
                for f in files:
                    out += BT.Executer('exiv2 -g City -g DateTime {f}', vars())

            for num in ['0', '10', '1000']:
                out     += BT.Executer('iotest s0 s1 s2 {server_url}/table.jpg {num}', vars())
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
            ,'table.jpg'
        ]

        pass_count  = 0
        fail_count  = 0
        out = BT.Output()

        out += '\n--- Read tests ---'
        for i in test_files:
            if BT.printTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out        += 'Failed: ' + i

        out += '\n--- Remove tests ---'
        for i in test_files:
            if BT.removeTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out        += 'Failed: ' + i

        out += '\n--- Add/Mod tests ---'
        for i in test_files:
            if BT.addModTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out        += 'Failed: ' + i

        out += '\n--- Extended tests ---'
        for i in test_files:
            if BT.extendedTest(i):
                pass_count += 1
            else:
                fail_count += 1
                out        += 'Failed: ' + i

        out += '\n--------------------\n'
        out += '{} passed, {} failed\n'.format(pass_count, fail_count)
        if fail_count:
            raise RuntimeError(str(out) + '\n' + BT.log.to_str())


    def iso65k_test(self):
        # test for ISOs which follow Annex G of EXIF 2.3 spec, i.e. ISOs,
        # which cannot be represented by Exif.Photo.ISOSpeedRatings due to
        # being larger than 65k
        out      = BT.Output()

        # Checks for old way of ISO readout based on the 16bit value
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to something <65k
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0001'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 60001' {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                      {filename}", vars())
        out     += ''

        # Old ISO is read out first, so if it doesn't indicate that
        # some higher ISO is used, the 16bit value should be returned,
        # ignoring the other tags (for now)
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to something <65k
        # - Exif.Photo.SensitivityType being set to "REI"
        # - Exif.Photo.RecommendedExposureIndex being set to != ISOSpeedRatings
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0002'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 60002'           {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 2'               {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.RecommendedExposureIndex 444444' {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                                {filename}", vars())
        out     += ''

        # Corner case check (highest ISO value not indicating possible
        # 16bit overflow in ISO)
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65534
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0003'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65534' {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                      {filename}", vars())
        out     += ''

        # Corner case check (ISO value indicating possible overflow,
        # but no additional informations available)
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65535
        # - Exif.Photo.SensitivityType NOT SET
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0004'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535' {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                      {filename}", vars())
        out     += ''

        # possible ISO value overflow, but additional information not valid
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65535
        # - Exif.Photo.SensitivityType being set to 0
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0005'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535' {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 0'     {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                      {filename}", vars())
        out     += ''

        # possible ISO value overflow, but additional information not valid
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65535
        # - Exif.Photo.SensitivityType being set to 8
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0006'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535' {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 8'     {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                      {filename}", vars())
        out     += ''

        # possible ISO value overflow, but additional information partially valid
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65535
        # - Exif.Photo.SensitivityType being set to 2 ("REI")
        # - Exif.Photo.RecommendedExposureIndex NOT SET
        # output:
        # - value of Exif.Photo.ISOSpeedRatings
        num      = '0007'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535' {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 2'     {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                      {filename}", vars())
        out     += ''

        # ISO value overflow, REI contains same value as 16bit ISO, though
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65535
        # - Exif.Photo.SensitivityType being set to 2 ("REI")
        # - Exif.Photo.RecommendedExposureIndex set to 65530
        # output:
        # - value of Exif.Photo.RecommendedExposureIndex
        num      = '0008'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'           {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 2'               {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.RecommendedExposureIndex 65530'  {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                                {filename}", vars())
        out     += ''

        # ISO value overflow, REI contains 16bit ISO value +1
        # input:
        # - Exif.Photo.ISOSpeedRatings being set to 65535
        # - Exif.Photo.SensitivityType being set to 2 ("REI")
        # - Exif.Photo.RecommendedExposureIndex set to 65536
        # output:
        # - value of Exif.Photo.RecommendedExposureIndex
        num      = '0009'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'           {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 2'               {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.RecommendedExposureIndex 65536'  {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                                {filename}", vars())
        out     += ''

        # old ISO not set
        # input:
        # - Exif.Photo.ISOSpeedRatings is NOT SET
        # - Exif.Photo.SensitivityType being set to 2 ("REI")
        # - Exif.Photo.RecommendedExposureIndex set to <65k
        # output:
        # - value of Exif.Photo.RecommendedExposureIndex
        num      = '0010'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 2'               {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.RecommendedExposureIndex 60010'  {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                                {filename}", vars())
        out     += ''

        # old ISO not set
        # input:
        # - Exif.Photo.ISOSpeedRatings is NOT SET
        # - Exif.Photo.SensitivityType being set to 2 ("REI")
        # - Exif.Photo.RecommendedExposureIndex set to >65k
        # output:
        # - value of Exif.Photo.RecommendedExposureIndex
        num      = '0011'
        filename = 'exiv2-iso65k-{}.jpg'.format(num)
        BT.copyTestFile('exiv2-empty.jpg', filename)
        out     += '------> iso65k test {} <-------'.format(num)
        out     += BT.Executer("exiv2 -M'set Exif.Photo.SensitivityType 2'               {filename}", vars())
        out     += BT.Executer("exiv2 -M'set Exif.Photo.RecommendedExposureIndex 100011' {filename}", vars())
        out     += BT.Executer("exiv2 -ps                                                {filename}", vars())
        out     += ''

        BT.reportTest('iso65k-test', out)


    def modify_test(self):
        # Test driver for write unit tests to build Exif metadata from scratch
        for i in ['exiv2-empty.jpg', 'exiv2-gc.jpg', 'modifycmd1.txt', 'modifycmd2.txt']:
            BT.copyTestFile(i)
        out  = BT.Output()
        out += BT.Executer('exiv2 -v -m modifycmd1.txt exiv2-empty.jpg')
        out += BT.Executer('exiv2 -v -m modifycmd2.txt exiv2-gc.jpg')
        out += BT.Executer('exiv2 -v -pi exiv2-empty.jpg')
        out += BT.Executer('exiv2 -v -pt exiv2-empty.jpg exiv2-gc.jpg')
        BT.reportTest('modify-test', out)


    def nls_test(self):
        # Test driver for exiv2.exe nls support
        nls          = BT.Executer('exiv2 -vVg nls').stdout.split('\n')[1]
        platform     = BT.Executer('exiv2 -vVg platform').stdout.split('\n')[1]

        if nls      != 'enable_nls=1':
            print('Skipped. Because exiv2 is not built with nls.')
            return

        if platform == 'platform=windows':
            print('Skipped. Because nls_test cannot be run msvc builds.')
            return

        if platform == 'platform=linux':
            LANG = 'LANGUAGE'
        else:
            LANG = 'LANG'

        share_dir    = os.path.normpath(os.path.join(BT.Config.bin_dir, '..', 'share2'))
        os.makedirs(share_dir, exist_ok=True)

        locale_dir   = '/usr/local/share/locale'
        if os.path.isdir(locale_dir) and os.path.isdir(share_dir):
            BT.cp(locale_dir, share_dir)
        else:
            print('Skipped. Because localisation files are not installed in {}.'.format(locale_dir))

        # The above part is checking the environment, and the following part is executing the actual test
        out      = BT.Output()
        for language in ['fr_FR', 'es_ES']:
            out += BT.Executer('exiv2', extra_env={'LC_ALL': language, LANG: language}, assert_returncode=[1])
        BT.reportTest('nls-test', out)


    def path_test(self):
        # Mini test-driver for path utility functions
        BT.copyTestFile('path-test.txt')
        out      = BT.Output()
        out     += BT.Executer('path-test path-test.txt')
        # print(out)


    def preview_test(self):
        # Test driver for previews
        images = [
            'exiv2-bug443.jpg'
            ,'exiv2-bug444.jpg'
            ,'exiv2-bug445.jpg'
            ,'exiv2-bug447.jpg'
            ,'exiv2-bug501.jpg'
            ,'exiv2-bug528.jpg'
            ,'exiv2-canon-eos-20d.jpg'
            ,'exiv2-canon-eos-300d.jpg'
            ,'exiv2-canon-eos-d30.jpg'
            ,'exiv2-canon-powershot-a520.jpg'
            ,'exiv2-canon-powershot-s40.crw'
            ,'exiv2-fujifilm-finepix-s2pro.jpg'
            ,'exiv2-gc.jpg'
            ,'exiv2-kodak-dc210.jpg'
            ,'exiv2-nikon-d70.jpg'
            ,'exiv2-nikon-e950.jpg'
            ,'exiv2-nikon-e990.jpg'
            ,'exiv2-olympus-c8080wz.jpg'
            ,'exiv2-panasonic-dmc-fz5.jpg'
            ,'exiv2-photoshop.psd'
            ,'exiv2-pre-in-xmp.xmp'
            ,'exiv2-sigma-d10.jpg'
            ,'exiv2-sony-dsc-w7.jpg'
            ,'glider.exv'
            ,'imagemagick.pgf'
            ,'iptc-psAPP13-noIPTC-psAPP13-wIPTC.jpg'
            ,'iptc-psAPP13-noIPTC.jpg'
            ,'iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg'
            ,'iptc-psAPP13-wIPTC1-psAPP13-wIPTC2.jpg'
            ,'iptc-psAPP13-wIPTCbeg.jpg'
            ,'iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg'
            ,'iptc-psAPP13-wIPTCempty.jpg'
            ,'iptc-psAPP13-wIPTCend.jpg'
            ,'iptc-psAPP13-wIPTCmid.jpg'
            ,'iptc-psAPP13-wIPTCmid1-wIPTCempty-wIPTCmid2.jpg'
            ,'smiley2.jpg'
        ]
        out         = BT.Output()
        report      = BT.Output()
        pass_count  = 0
        fail_count  = 0
        preview_dir = os.path.join(BT.Config.data_dir, 'preview')

        for filename in images:
            image   = filename.split('.')[0]
            BT.copyTestFile(filename)
            out    += '\n-----> {} <-----\n'.format(filename)

            out    += 'Command: exiv2 -pp ' + filename
            e       = BT.Executer('exiv2 -pp {filename}', vars(), assert_returncode=None, redirect_stderr_to_stdout=False)
            out    += e.stdout
            out    += 'Exit code: {}'.format(e.returncode)
            BT.rm(*BT.find(pattern=image + '-preview*'))

            out    += '\nCommand: exiv2 -f -ep ' + filename
            e       = BT.Executer('exiv2 -f -ep {filename}', vars(), assert_returncode=None, redirect_stderr_to_stdout=False)
            out    += e.stdout
            out    += 'Exit code: {}'.format(e.returncode)

            # Check the difference
            e       = BT.Executer('exiv2 -pp {filename}', vars(), assert_returncode=None, redirect_stderr_to_stdout=False)
            preview_num         = e.stdout[:e.stdout.find(':')].lstrip('Preview ')
            for test_file in BT.find(pattern='{image}-preview{preview_num}.*'.format(**vars())):
                reference_file  = os.path.join(preview_dir, test_file)
                if BT.diffCheck(reference_file, test_file, in_bytes=True):
                    pass_count += 1
                else:
                    fail_count += 1
                    report     += 'Failed: ' + filename

        report  += '\n{} passed, {} failed\n'.format(pass_count, fail_count)
        if fail_count:
            raise RuntimeError('\n' + str(report) + '\n' + BT.log.to_str())

        BT.reportTest('preview-test', out)


    def stdin_test(self):
        return # temporarily disable
        # Test driver for stdin
        try:
            import lxml
        except ModuleNotFoundError:
            print('Skipped. Because it misses module lxml. Please install: `pip install lxml`')
            return

        out     = BT.Output()
        a       = 'exiv2-bug1229.jpg'  # jpg with 2 APP1/xap segments
        b       = 'girl.jpg'
        BT.copyTestFile(a)

        def get_xmpData(img):
            e    = BT.Executer('exiv2 -pX   {img}', vars(), decode_output=False)
            return e.stdout.replace(b'\n', b'\r\n', 16) # Ignore the difference in newline

        BT.copyTestFile(a, b)
        out     += BT.Executer('exiv2 -pS   {b}', vars())
        out     += BT.Executer('exiv2 -dX   {b}', vars())  # remove first
        out     += BT.Executer('exiv2 -pS   {b}', vars())

        e = BT.Executer('exiv2 -pX   {a}', vars(), decode_output=False)
        with open('out2', 'wb') as f:
            f.write(e.stdout)

        out     += BT.Executer('exiv2 -iXX- {b}', vars(), stdin=get_xmpData(a))
        out     += BT.Executer('exiv2 -pS   {b}', vars())

        BT.copyTestFile(a, b)
        out     += BT.Executer('exiv2 -dX   {b}', vars())
        out     += BT.Executer('exiv2 -dX   {b}', vars())
        out     += BT.Executer('exiv2 -pS   {b}', vars())
        out     += BT.Executer('exiv2 -iXX- {b}', vars(), stdin=get_xmpData(a))
        out     += BT.Executer('exiv2 -pS   {b}', vars())

        for f in ['Reagan.jpg', 'Reagan.tiff', 'ReaganSmallPng.png']:
            BT.copyTestFile(f)
            out     += BT.Executer('exiv2 -iXX- {f}', vars(), stdin=get_xmpData(a))
            e        = BT.Executer('exiv2 -pX   {f}', vars())
            out     += """
<?xml version="1.0"?>
<?xpacket begin='\ufeff' id='W5M0MpCehiHzreSzNTczkc9d'?>
{}
<?xpacket end='w'?>
""".strip('\n').format(BT.pretty_xml(e.stdout))

        for f in ['Reagan.jpg', 'ReaganSmallPng.png']:
            BT.copyTestFile(f)
            BT.copyTestFile(a, b)
            out     += BT.Executer('exiv2 -pS   {b}', vars())
            e        = BT.Executer('exiv2 -ea-  {f}', vars(), decode_output=False)
            out     += BT.Executer('exiv2 -ia-  {b}', vars(), stdin=e.stdout)
            out     += BT.Executer('exiv2 -pS   {b}', vars())

        BT.copyTestFile('Reagan.tiff')   # 1272 ReaganLargeTiff.tiff
        for img in ['Reagan.jpg', 'ReaganSmallPng.png', 'exiv2-bug1199.webp']:
            BT.copyTestFile(img)
            e        = BT.Executer('exiv2 -eC-  Reagan.tiff', decode_output=False)

            # Ignore the difference in the path separator
            stdout = e.stdout
            for pair in [
                (b'\x03/\x9e', b'\x03\\\x9e'),
                (b'\x0c/\x0c', b'\x0c\\\x0c'),
                (b'V/V'      , b'V\\V'      ),
                (b'\xe5/5'   , b'\xe5\\5'   ),
                (b'5/\x86'   , b'5\\\x86'   ),
                (b'\x86/\xd6', b'\x86\\\xd6'),
                (b'\xac/\xac', b'\xac\\\xac'),
                (b'\xd7/\xd7', b'\xd7\\\xd7'),
            ]:
                stdout  = stdout.replace(pair[0], pair[1])

            out     += BT.Executer('exiv2 -iC-  {img}', vars(), stdin=stdout)
            out     += BT.Executer('exiv2 -pS   {img}', vars())
            if img == 'ReaganSmallPng.png':
                with open('out2', 'wb') as f:
                    f.write(e.stdout)

        BT.reportTest('stdin-test', out)


    def stringto_test(self):
        # Test driver for tests of stringToLong/Float/Rational
        out      = BT.Output()
        out     += BT.Executer('stringto-test')
        BT.reportTest('stringto-test', out)


    def tiff_test(self):
        # TIFF parser test driver

        def exifprobe(img):
            out  = BT.Output()

            # Convert each line from that format:
            #   Exif.Image.NewSubfileType       Long    1  Primary image
            # to that format:
            #   NewSubfileType 	 Long 	 1 	 "Primary image"
            tags = []
            typs = []
            lens = []
            vals = []
            e    = BT.Executer('exiv2 -pa {img}', vars(), redirect_stderr_to_stdout=False)
            for line in e.stdout.split('\n'):
                fields = re.findall(r'^(\S*)\s*(\S*)\s*(\d*)\s*(.*)$', line)[0]
                tags  += [fields[0].split('.')[2]]      # tag
                typs  += [fields[1]]                    # type
                lens  += [fields[2]]                    # length
                vals  += [fields[3]]                    # value

            out += 'exiv2 -pa output'
            for i in range(len(tags)):
                out += ' \t '.join([tags[i], typs[i], lens[i], '"{}"'.format(vals[i])])

            # Parse -pS output:
            # address |    tag                           |      type |    count |   offset | value
            #     254 | 0x00fe NewSubfileType            |      LONG |        1 |        0 | 0
            TAGS = []
            TYPS = []
            LENS = []
            VALS = []
            e    = BT.Executer('exiv2 -pS {img}', vars(), redirect_stderr_to_stdout=False)
            for line in e.stdout.split('\n')[2:-1]:
                fields = [i.strip(' ') for i in line.split('|')]
                TAGS  += [fields[1].split(' ')[1]]
                TYPS  += [fields[2]]
                LENS  += [fields[3]]
                VALS  += [fields[5]]

            out += '\nexiv2 -pS output'
            for i in range(len(TAGS)):
                out += ' \t '.join([TAGS[i], TYPS[i], LENS[i], '"{}"'.format(VALS[i])])

            out += '\nAnalysis'
            out += 'count =  {} COUNT =  {}'.format(len(tags), len(TAGS))

            # Make them have the same number of lines
            max_lines  = max(len(TAGS), len(tags))
            for _list in [tags, typs, lens, vals, TAGS, TYPS, LENS, VALS]:
                _list += [''] * (max_lines - len(_list))

            # Compare the main fields of each line
            for i in range(max_lines):
                if TAGS[i]  != tags[i]:
                    out     += 'TAG {} {} mismatch'.format(TAGS[i], tags[i]).replace('  ', ' ')

                TYPS[i]      = TYPS[i].upper()
                typs[i]      = typs[i].upper()
                if TYPS[i]  != typs[i]:
                    out     += 'TYPE {} {} mismatch'.format(TYPS[i], typs[i]).replace('  ', ' ')

                if LENS[i]  != lens[i]:
                    out     += 'Length {} {} mismatch'.format(LENS[i], lens[i]).replace('  ', ' ')

                if typs[i]  == 'ASCII' and VALS[i] != vals[i]:
                    out     += 'Value {} {} mismatch'.format(VALS[i], vals[i]).replace('  ', ' ')

            return str(out)

        test_file = 'mini9.tif'
        BT.copyTestFile(test_file)
        out      = BT.Output()
        out     += exifprobe(test_file)
        out     += BT.Executer('tiff-test {test_file}', vars())
        out     += exifprobe(test_file)
        BT.reportTest('tiff-test', out)


    def version_test(self):
        # Test driver for exiv2 --verbose --version
        out      = BT.Output()
        out     += BT.Executer('exiv2 --verbose --version')


    def webp_test(self):
        return # temporarily disable

        # Test driver for webp
        webp     = 'exiv2-bug1199.webp' # http://dev.exiv2.org/attachments/download/1033/Stonehenge-with-icc.webp
        icc      = 'exiv2-bug1199.icc'
        exv      = 'exiv2-bug1199.exv'
        xmp      = 'exiv2-bug1199.xmp'
        tiff     = 'Reagan.tiff'
        out      = BT.Output()

        # Extract the XMP
        BT.copyTestFile(webp)
        out     += BT.Executer('exiv2 -pS {webp}', vars())
        e        = BT.Executer('exiv2 -pX {webp}', vars())
        out     += """
<?xml version="1.0"?>
<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>
{}
<?xpacket end="w"?>
""".strip('\n').format(BT.pretty_xml(e.stdout))

        # Test deleting metadata
        for option in ['-dC', '-de', '-dx', '-dCe', '-dCx', '-dCxe']:
            BT.copyTestFile(webp)
            out     += BT.Executer('exiv2 -pS      {webp}', vars())
            out     += BT.Executer('exiv2 {option} {webp}', vars())
            out     += BT.Executer('exiv2 -pS      {webp}', vars())

        # Extract the icc
        BT.copyTestFile(webp)
        BT.copyTestFile(tiff)
        out     += BT.Executer('exiv2 -pS {webp}', vars())
        BT.save(   BT.Executer('exiv2 -pC {tiff}', vars(), decode_output=False).stdout, icc)
        out     += BT.Executer('exiv2 -iC {webp}', vars())
        out     += BT.Executer('exiv2 -pS {webp}', vars())

        # Copy the XMP from the test file
        BT.copyTestFile(webp)
        BT.save(   BT.Executer('exiv2 -pX {webp}', vars(), decode_output=False).stdout, xmp)
        out     += BT.Executer('exiv2 -ea --force {webp}', vars())

        BT.copyTestFile(webp)
        out     += BT.Executer('exiv2 -pS  {webp}', vars())
        out     += BT.Executer('exiv2 -iXX {webp}', vars())
        out     += BT.Executer('exiv2 -pS  {webp}', vars())
        out     += BT.Executer('exiv2 -ix  {webp}', vars())

        # Copy the XMP from Reagan.tiff to test file
        BT.copyTestFile(tiff)
        BT.save(   BT.Executer('exiv2 -pX  {tiff}', vars(), decode_output=False).stdout, xmp)
        out     += BT.Executer('exiv2 -ea --force  {tiff}', vars())
        BT.mv('Reagan.exv', exv)

        BT.copyTestFile(webp)
        out     += BT.Executer('exiv2 -pS   {webp}', vars())
        out     += BT.Executer('exiv2 -iXX  {webp}', vars())
        out     += BT.Executer('exiv2 -pS   {webp}', vars())
        out     += BT.Executer('exiv2 -ix   {webp}', vars())

        # Copy the XMP from exiv2-bug922.jpg to test file
        BT.copyTestFile('exiv2-bug922.jpg')
        BT.save(   BT.Executer('exiv2 -pX  exiv2-bug922.jpg', decode_output=False).stdout, xmp)
        BT.Executer(           'exiv2 -ea --force  exiv2-bug922.jpg')
        BT.mv('exiv2-bug922.exv', exv)

        BT.copyTestFile(webp)
        out     += BT.Executer('exiv2 -pS   {webp}', vars())
        out     += BT.Executer('exiv2 -ix   {webp}', vars())
        out     += BT.Executer('exiv2 -pS   {webp}', vars())
        out     += BT.Executer('exiv2 -iXX  {webp}', vars())
        out     += BT.Executer('exiv2 -pS   {webp}', vars())

        BT.copyTestFile('exiv2-bug922.jpg', webp)
        out     += BT.Executer('exiv2 --force -ea  {webp}', vars())
        BT.copyTestFile(webp)
        out     += BT.Executer('exiv2 -pS   {webp}', vars())
        out     += BT.Executer('exiv2 -ie   {webp}', vars())
        out     += BT.Executer('exiv2 -pS   {webp}', vars())

        BT.reportTest('webp-test', out)


    def write_test(self):
        # Test driver for the write unit tests
        images = [
            'exiv2-canon-powershot-s40.jpg'
            ,'exiv2-kodak-dc210.jpg'
            ,'exiv2-fujifilm-finepix-s2pro.jpg'
            ,'exiv2-sigma-d10.jpg'
            ,'exiv2-nikon-e990.jpg'
            ,'exiv2-nikon-d70.jpg'
            ,'exiv2-nikon-e950.jpg'
        ]
        for i in images:
            BT.copyTestFile(i)

        out  = BT.Output()
        out += BT.runTestCase( 1, 'exiv2-canon-powershot-s40.jpg')
        out += BT.runTestCase( 2, 'exiv2-canon-powershot-s40.jpg')
        out += BT.runTestCase( 3, 'exiv2-kodak-dc210.jpg')
        out += BT.runTestCase( 4, 'exiv2-canon-powershot-s40.jpg')
        out += BT.runTestCase( 5, 'exiv2-canon-powershot-s40.jpg')
        out += BT.runTestCase( 6, 'exiv2-kodak-dc210.jpg')
        out += BT.runTestCase( 7, 'exiv2-fujifilm-finepix-s2pro.jpg')
        out += BT.runTestCase( 8, 'exiv2-sigma-d10.jpg')
        out += BT.runTestCase( 9, 'exiv2-nikon-e990.jpg')
        out += BT.runTestCase(10, 'exiv2-nikon-e950.jpg')
        out += BT.runTestCase(11, 'exiv2-nikon-d70.jpg')

        # Adjust the output to be compatible with the reference output
        out = str(out)
        for img in images:
            out = out.replace('Reading file ' + img, 'Reading file ./' + img)

        # Ignore output differences between BT.diff() and GNU dIff
        for pair in [
            ('24,2c24,2', '24,25c24,25'),
            ('29,2c29,2', '29,30c29,30'),
            ('27,2c27,2', '27,28c27,28'),
            ('28,2c28,2', '28,29c28,29'),
        ]:
            out  = out.replace(pair[0], pair[1])

        BT.reportTest('write-test', out)


    def write2_test(self):
        # Test driver for write unit tests to build Exif metadata from scratch
        img      = 'exiv2-empty.jpg'
        BT.copyTestFile(img)
        out      = BT.Output()
        out     += BT.Executer('write2-test {img}', vars())
        BT.reportTest('write2-test', out)


    def xmpparser_test(self):
        # XMP parser test driver
        images = ['BlueSquare.xmp', 'StaffPhotographer-Example.xmp', 'xmpsdk.xmp']
        out      = BT.Output()

        for img in images:
            BT.copyTestFile(img)
            out += BT.Executer('xmpparser-test {img}', vars())
            out += BT.diff(img, img + '-new')

        xmp = 'xmpsdk.xmp'
        BT.save(BT.Executer('xmpparse {xmp}'    , vars()).stdout, 't1')
        BT.save(BT.Executer('xmpparse {xmp}-new', vars()).stdout, 't2')
        out += BT.diff('t1', 't2')

        out += BT.Executer('xmpsample')
        for img in ['exiv2-empty.jpg', 'cmdxmp.txt']:
            BT.copyTestFile(img)
        out += BT.Executer('exiv2 -v -m cmdxmp.txt exiv2-empty.jpg', assert_returncode=[0, 1])
        out += BT.Executer('exiv2 -v -px exiv2-empty.jpg')

        # Ignore output differences between BT.diff() and GNU dIff
        out = str(out)
        out  = out.replace("""
34,0c35
---
>      <rdf:li xml:lang="x-default">Blue Square Test File - .jpg</rdf:li>
36c36,0
<      <rdf:li xml:lang="x-default">Blue Square Test File - .jpg</rdf:li>
67,21c67,21
""".strip('\n'),
"""
35d34
<      <rdf:li xml:lang="en-US">Blue Square Test File - .jpg</rdf:li>
36a36
>      <rdf:li xml:lang="en-US">Blue Square Test File - .jpg</rdf:li>
67,87c67,87
""".strip('\n'))
        for pair in [
            ('46,0c47\n---'             , '46a47'),
            ('160,32c161'               , '160,191c161'),
            ('1,49c1,65'                , '1,48c1,65'),
            ('<     </rdf:RDF>\n< '     , '<     </rdf:RDF>'),
            ('> <?xpacket end="w"?>\n'  , '> <?xpacket end="w"?>\n/ No newline at end of file'),
        ]:
            out  = out.replace(pair[0], pair[1])

        BT.reportTest('xmpparser-test', out)


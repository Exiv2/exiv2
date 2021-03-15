import os
import shutil
import sys
import urllib

from .common import *
from .config import Config
from .executer import Executer


def copyTestFile(src, dst=''):
    """ Copy one test file from data_dir to tmp_dir """
    if not dst:
        dst = src
    shutil.copy(os.path.join(Config.data_dir, src),
                os.path.join(Config.tmp_dir, dst))


def reportTest(testname, output: str, encoding='utf-8'):
    """ If the output of the test case is correct, this function returns None. Otherwise print its error. """
    output               = str(output) + '\n'
    encoding             = encoding or Config.encoding
    reference_file       = os.path.join(Config.data_dir, '{}.out'.format(testname))
    reference_output     = cat(reference_file, encoding=encoding)
    if reference_output == output:
        return
    log.error('The output of the testcase mismatch the reference')
    output_file = os.path.join(Config.tmp_dir, '{}.out'.format(testname))
    save(output, output_file, encoding=encoding)
    log.info('The output has been saved to file {}'.format(output_file))
    log.info('simply_diff:\n' + str(simply_diff(reference_file, output_file, encoding=encoding)))
    raise RuntimeError('\n' + log.dump())


def ioTest(filename):
    src     = os.path.join(Config.data_dir, filename)
    out1    = os.path.join(Config.tmp_dir, '{}.1'.format(filename))
    out2    = os.path.join(Config.tmp_dir, '{}.2'.format(filename))
    Executer('iotest {src} {out1} {out2}', vars())
    assert md5sum(src) == md5sum(out1), 'The output file is different'
    assert md5sum(src) == md5sum(out2), 'The output file is different'


def eraseTest(filename):
    test_file   = filename + '.etst'
    good_file   = os.path.join(Config.data_dir, filename + '.egd')
    copyTestFile(filename, test_file)
    Executer('metacopy {test_file} {test_file}', vars())
    return diffCheck(good_file, test_file, in_bytes=True)


def copyTest(num, src, good):
    test_file   = '{}.c{}tst'.format(good, num)
    src_file    = os.path.join(Config.data_dir, src)
    good_file   = os.path.join(Config.data_dir, '{}.c{}gd'.format(good, num))
    copyTestFile(good, test_file)
    Executer('metacopy -a {src_file} {test_file}', vars())
    return diffCheck(good_file, test_file, in_bytes=True)


def iptcTest(num, src, good):
    test_file   = '{}.i{}tst'.format(good, num)
    src_file    = os.path.join(Config.data_dir, src)
    good_file   = os.path.join(Config.data_dir, '{}.i{}gd'.format(good, num))
    copyTestFile(good, test_file)
    Executer('metacopy -ip {src_file} {test_file}', vars())
    return diffCheck(good_file, test_file, in_bytes=True)


def printTest(filename):
    test_file   = filename + '.iptst'
    src_file    = os.path.join(Config.data_dir, filename)
    good_file   = os.path.join(Config.data_dir, filename + '.ipgd')
    copyTestFile(filename, test_file)

    e           = Executer('iptcprint {src_file}', vars(), assert_returncode=None, decode_output=False)
    stdout      = e.stdout.replace(Config.data_dir.replace(os.path.sep, '/').encode(), b'../data') # Ignore the difference of data_dir on Windows
    save(stdout + b'\n', test_file)

    return diffCheck(good_file, test_file, in_bytes=True)


def removeTest(filename):
    tmp         = 'temp'
    test_file   = filename + '.irtst'
    src_file    = os.path.join(Config.data_dir, filename)
    good_file   = os.path.join(Config.data_dir, filename + '.irgd')
    copyTestFile(filename, tmp)
    stdin       = """
r Iptc.Application2.Byline
r Iptc.Application2.Caption
r Iptc.Application2.Keywords
r Iptc.Application2.Keywords
r Iptc.Application2.Keywords
r Iptc.Application2.CountryName
""".lstrip('\n').encode()
    Executer('iptctest {tmp}', vars(), stdin=stdin)
    e           = Executer('iptcprint {tmp}', vars(), assert_returncode=None, decode_output=False)
    save(e.stdout + b'\n', test_file)
    return diffCheck(good_file, test_file, in_bytes=True)


def addModTest(filename):
    tmp         = 'temp'
    test_file   = filename + '.iatst'
    src_file    = os.path.join(Config.data_dir, filename)
    good_file   = os.path.join(Config.data_dir, filename + '.iagd')
    copyTestFile(filename, tmp)
    stdin       = """
a Iptc.Application2.Headline		  The headline I am
a Iptc.Application2.Keywords		  Yet another keyword
m Iptc.Application2.DateCreated		  2004-8-3
a Iptc.Application2.Urgency			  3
m Iptc.Application2.SuppCategory	  "bla bla ba"
a Iptc.Envelope.ModelVersion		  2
a Iptc.Envelope.TimeSent			  14:41:0-05:00
a Iptc.Application2.RasterizedCaption 230 42 34 2 90 84 23 146
""".lstrip('\n').encode()
    Executer('iptctest {tmp}', vars(), stdin=stdin)
    e           = Executer('iptcprint {tmp}', vars(), assert_returncode=None, decode_output=False)
    save(e.stdout + b'\n', test_file)
    return diffCheck(good_file, test_file, in_bytes=True)


def extendedTest(filename):
    tmp         = 'temp'
    test_file   = filename + '.ixtst'
    src_file    = os.path.join(Config.data_dir, filename)
    good_file   = os.path.join(Config.data_dir, filename + '.ixgd')
    copyTestFile(filename, tmp)
    stdin       = cat(os.path.join(Config.data_dir, 'ext.dat'))
    Executer('iptctest {tmp}', vars(), stdin=stdin)
    e           = Executer('iptcprint {tmp}', vars(), decode_output=False)
    save(e.stdout + b'\n', test_file)
    return diffCheck(good_file, test_file, in_bytes=True)


def runTestCase(num, img):
    """ Run the requested test case number with the given image """
    out_img     = 'test{}.jpg'.format(num)
    thumb_jpg   = 'thumb{}.jpg'.format(num)
    thumb_tif   = 'thumb{}.tif'.format(num)
    rm(out_img, thumb_jpg, thumb_tif)
    rm('iii', 'ttt')
    cp(img, out_img)
    out  = Output()
    out += '------------------------------------------------------------'

    e    = Executer('exifprint {img}', vars(), redirect_stderr_to_stdout=False, decode_output=False)
    out += e.stderr.decode() if e.stderr else None
    save(e.stdout, 'iii')

    e    = Executer('write-test {img} {num}', vars(), redirect_stderr_to_stdout=False, decode_output=False)
    out += e.stderr.decode() if e.stderr else None
    save(e.stdout, 'ttt')

    out += diff('iii', 'ttt')
    return str(out)


def verbose_version(verbose=False):
    """ Get the key-value pairs of Exiv2 verbose version.  """
    vv    = {}
    lines = Executer('exiv2 --verbose --version').stdout.split('\n')
    for line in lines:
        kv = line.rstrip().split('=')
        if len(kv)  == 2:
            key, val = kv
            if not key in vv:
                vv[key] = val
            elif isinstance(vv[key], list):
                vv[key].append(val)
            else:
                vv[key] = [vv[key]]
    if verbose:
        for key in vv:
            val = vv[key]
            if isinstance(val, list):
                val = '[ {}   +{} ]'.format(val[0], len(val) - 1)
            print(key.ljust(20), val)
    return vv


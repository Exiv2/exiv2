
"""
Here are some code that are highly coupled to test cases.
"""
import os
import shlex
import shutil
import subprocess
import sys
import urllib

from .common_code import *


def copyTestFile(src, dst=''):
    """ Copy one test file from data_dir to tmp_dir """
    if not dst:
        dst = src
    shutil.copy(os.path.join(Config.data_dir, src),
                os.path.join(Config.tmp_dir, dst))


def diffCheck(file1, file2, in_bytes=False, encoding='utf-8'):
    """ Compare two files to see if they are different """
    if in_bytes:
        d = diff_bytes(file1, file2, return_str=True)
        if d:
            log.info('diff_bytes:\n' + d)
    else:
        d = diff(file1, file2, encoding=encoding)
        if d:
            log.info('diff:\n' + d)
    return d == ''


def simply_diff(file1, file2, encoding='utf-8'):
    """ Find the first different line of the two text files """
    encoding    = encoding
    list1       = cat(file1, encoding=encoding).split('\n')
    list2       = cat(file2, encoding=encoding).split('\n')
    if list1   == list2:
        return

    report      = []
    report     += ['{}: {} lines'.format(file1, len(list1))]
    report     += ['{}: {} lines'.format(file2, len(list2))]

    # Make them have the same number of lines
    max_lines   = max(len(list1), len(list2))
    for _list in [list1, list2]:
        _list  += [''] * (max_lines - len(_list))

    # Compare each line
    for i in range(max_lines):
        if list1[i] != list2[i]:
            report  += ['The first mismatch is in line {}:'.format(i + 1)]
            report  += ['< {}'.format(list1[i])]
            report  += ['> {}'.format(list2[i])]
            break
    return '\n'.join(report)


class Executer:
    """
    Execute a command in the shell, return a `Executer` object.
    - Compatible with Windows, Linux, MacOS and other platforms.
    - If a binary of Exiv2 is executed, the absolute path is automatically added.
    - `compatible_output=True`: filter out path delimiters, whitespace characters in output
    - `decode_output=True`: decode output from bytes to str

    Sample:
    >>> Executer('echo Hello').stdout
    >>> Executer('exiv2 --help').stdout
    """

    def __init__(self, cmd: str,
                 vars_dict=dict(),
                 cwd=None,
                 extra_env=dict(),
                 encoding='utf-8',
                 stdin: (str, bytes) = None,
                 redirect_stderr_to_stdout=True,
                 assert_returncode=[0],
                 compatible_output=True,
                 decode_output=True):
        self.cmd            = cmd.format(**vars_dict)
        self.cwd            = cwd or Config.tmp_dir

        # set environment variables
        self.env            = os.environ.copy()
        self.env.update({'DYLD_LIBRARY_PATH': Config.dyld_library_path})
        self.env.update({'LD_LIBRARY_PATH': Config.ld_library_path})
        self.env.update({'TZ': 'GMT-8'})
        self.env.update(extra_env)

        self.encoding       = encoding or Config.encoding
        self.stdin          = stdin
        # self.stdout       = None
        # self.stderr       = None
        self.redirect_stderr_to_stdout  = redirect_stderr_to_stdout
        self.assert_returncode          = assert_returncode
        # self.returncode   = 0
        self.compatible_output  = compatible_output
        self.decode_output      = decode_output

        # Generate the args for subprocess.Popen
        args = self.cmd.split(' ', maxsplit=1)
        if args[0] in Config.bin_files:
            args[0]     = os.path.join(Config.bin_dir, args[0])
        args            = ' '.join(args)
        if Config.platform == 'win32':
            self.args   = args.replace('\'', '\"')
        else:
            self.args   = shlex.split(args, posix=os.name == 'posix')

        if len(Config.valgrind) > 0:
            self.args = [ Config.valgrind ] + self.args

        # Check stdin
        if self.stdin:
            if not isinstance(stdin, bytes):
                self.stdin = str(stdin).encode(self.encoding)

        self.run()

    def run(self):
        # Check stdout
        if self.redirect_stderr_to_stdout:
            stderr   = subprocess.STDOUT
        else:
            stderr   = subprocess.PIPE

        # Execute the command in subprocess
        try:
            with subprocess.Popen(self.args, cwd=self.cwd, env=self.env,
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=stderr) as self.subprocess:
                try:
                    output  = self.subprocess.communicate(self.stdin, timeout=10)  # Assign (stdout, stderr) to output
                except subprocess.TimeoutExpired:
                    self.subprocess.kill()
                    output  = self.subprocess.communicate()
        except:
            raise RuntimeError('Failed to execute: {}'.format(self.args))
        output          = [i or b'' for i in output]
        output          = [i.rstrip(b'\r\n').rstrip(b'\n') for i in output] # Remove the last line break of the output

        # Extract stdout and stderr
        if self.compatible_output:
            output      = [i.replace(b'\r\n', b'\n')    for i in output]   # Fix dos line-endings
            output      = [i.replace(b'\\', rb'/')      for i in output]   # Fix dos path separators
        if self.decode_output:
            output      = [i.decode(self.encoding)      for i in output]
        self.stdout, self.stderr = [i or None           for i in output]

        # Check return code
        self.returncode = self.subprocess.returncode
        if self.assert_returncode and self.returncode not in self.assert_returncode:
            log.error('Failed to execute: {}'.format(self.args))
            log.error('The asserted return code is {}, but got {}'.format(str(self.assert_returncode), self.subprocess.returncode))
            log.info('OUTPUT:\n{}'.format(output[0] + output[1]))
            raise RuntimeError('\n' + log.dump())


class Output:
    """
    Simulate the stdout buffer.
    You can use `out+=x` to simulate `print(x)`

    Sample:
    >>> out = Output()
    >>> out
    >>> str(out)
    >>> out += 'Hello'
    >>> out += 1
    >>> out += ['Hi' , 2]
    >>> out += None         # no effect
    >>> str(out)
    """
    def __init__(self):
        self.lines = []
        self.newline = '\n'

    def __str__(self):
        return self.newline.join(self.lines)

    # Comment it so that log does not automatically convert to str type
    # def __repr__(self):
    #     return str(self)

    def __add__(self, other):
        if isinstance(other, Executer):
            other = other.stdout
        if other != None:
            self.lines.append(str(other))
        return self

    def __radd__(self, other):
        return self.__add__(other)


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



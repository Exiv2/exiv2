import difflib
import fnmatch
import hashlib
import multiprocessing
import os
import platform
import re
import shlex
import shutil
import subprocess
import time
from http import server
from urllib import request


"""
Part 1:
Here is the configuration part of test cases.
"""

class Config:
    # The configuration parameters for bash test
    # When you run the test cases through `python3 runner.py`, the function configure_suite() in system_tests.py will override these parameters.
    exiv2_dir   = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
    bin_dir     = os.path.join(exiv2_dir, 'build/bin')
    data_dir    = os.path.join(exiv2_dir, 'test/data')
    tmp_dir     = os.path.join(exiv2_dir, 'test/tmp')
    system_name = platform.system() or 'Unknown'
    exiv2_http  = 'http://127.0.0.1'
    exiv2_port  = 12760

    @classmethod
    def init(cls):
        """ Init test environments and variables that may be modified """
        os.makedirs(cls.tmp_dir, exist_ok=True)
        os.chdir(cls.tmp_dir)
        log.buffer      = []
        cls.bin_files   = [i.split('.')[0] for i in os.listdir(cls.bin_dir)]
        cls.encoding    = 'utf-8'


"""
Part 2:
Here are some common functions that are poorly coupled with test cases.
"""

def cp(src, dest):
    """ It is used to copy one file, cannot handle directories """
    shutil.copy(src, dest)


def mv(src, dest):
    """ It is used to move one file, cannot handle directories """
    shutil.move(src, dest)


def rm(*files):
    """ It is used to remove files, cannot handle directories """
    for i in files:
        try:
            os.remove(i)
        except FileNotFoundError:
            continue


def find(pattern=None, re_pattern=None, directory='.', depth=-1, onerror=print) -> list:
    """
    Find files that match the pattern in the specified directory and return their paths.
    - `pattern`     : Filter filename based on shell-style wildcards.
    - `re_pattern`  : Filter filename based on regular expressions.
    - `directory`   : Find files in this directory and its subdirectories
    - `depth`       : Depth of subdirectories. If its value is negative, the depth is infinite.
    - `onerror`     : A callable parameter. it will be called if an exception occurs.

    Work in recursive mode. If there are thousands of files, the runtime may be several seconds.

    Sample:
    >>> find(pattern='*.py')
    >>> find(re_pattern='.*.py')
    """
    if not os.path.isdir(directory):
        raise ValueError("{} is not an existing directory.".format(directory))

    try:
        file_list = os.listdir(directory)
    except PermissionError as e:    # Sometimes it does not have access to the directory
        onerror("PermissionError: {}".format(e))
        return -1

    path_list = []
    for filename in file_list:
        path = os.path.join(directory, filename)
        if depth != 0 and os.path.isdir(path):
            sub_list = find(path, depth-1, pattern, re_pattern, onerror)
            if sub_list != -1:
                path_list.extend(sub_list)
            continue
        if pattern and not fnmatch.fnmatch(filename, pattern):
            continue
        if re_pattern and not re.findall(re_pattern, filename):
            continue
        path_list.append(path)

    return path_list


def cat(*files, encoding=None, return_bytes=False):
    if return_bytes:
        result = b''
        for i in files:
            with open(i, 'rb') as f:
                result += f.read()
    else:
        result = ''
        for i in files:
            with open(i, 'r', encoding=encoding or Config.encoding) as f:
                result += f.read()
    return result


def grep(pattern, *files, encoding=None):
    result  = ''
    pattern = '.*{}.*'.format(pattern)
    for i in files:
        content = cat(i, encoding=encoding or Config.encoding)
        result += '\n'.join(re.findall(pattern, content))
    return result


def save(content: (bytes, str, tuple, list), filename, encoding=None):
    if isinstance(content, bytes):
        with open(filename, 'wb') as f:
            f.write(content)
        return
    if isinstance(content, (tuple, list)):
        content = '\n'.join(content)
    if isinstance(content, str):
        with open(filename, 'w', encoding=encoding or Config.encoding) as f:
            f.write(content)
    else:
        raise ValueError('Expect content of type (bytes, str, tuple, list), but get {}'.format(type(content).__name__))


def diff(file1, file2, encoding=None):
    """
    Simulates the output of GNU diff.
    You can use `diff(f1, f2)` to simulate `diff -w f1 f2`
    """
    encoding     = encoding or Config.encoding
    texts        = []
    for f in [file1, file2]:
        with open(f, encoding=encoding) as f:
            text = f.read()
        text     = text.replace('\r\n', '\n') # Ignore line breaks for Windows
        texts   += [text.split('\n')]
    text1, text2 = texts

    output       = []
    new_part     = True
    num          = 0
    for line in difflib.unified_diff(text1, text2, fromfile=file1, tofile=file2, lineterm=''):
        num     += 1
        if num   < 3:
            # line         = line.replace('--- ', '<<< ')
            # line         = line.replace('+++ ', '>>> ')
            # output      += [line]
            continue

        flag             = line[0]
        if flag         == '-':   # line unique to sequence 1
            new_flag     = '< '
        elif flag       == '+':   # line unique to sequence 2
            new_flag     = '> '
            if new_part:
                new_part = False
                output  += ['---']
        elif flag       == ' ':   # line common to both sequences
            # new_flag   = '  '
            continue
        elif flag       == '?':   # line not present in either input sequence
            new_flag     = '? '
        elif flag       == '@':
            output      += [re.sub(r'@@ -([^ ]+) \+([^ ]+) @@', r'\1c\2', line)]
            new_part     = True
            continue
        else:
            new_flag     = flag
        output          += [new_flag + line[1:]]

    return '\n'.join(output)


def diff_bytes(file1, file2, return_str=False):
    """
    Compare the bytes of two files.
    Simulates the output of GNU diff.
    """
    texts        = []
    for f in [file1, file2]:
        with open(f, 'rb') as f:
            text = f.read()
        text     = text.replace(b'\r\n', b'\n') # Ignore line breaks for Windows
        texts   += [text.split(b'\n')]
    text1, text2 = texts

    output       = []
    new_part     = True
    num          = 0
    for line in difflib.diff_bytes(difflib.unified_diff, text1, text2,
                                   fromfile=file1.encode(), tofile=file2.encode(), lineterm=b''):
        num     += 1
        if num   < 3:
            line         = line.decode()
            line         = line.replace('--- ', '<<< ')
            line         = line.replace('+++ ', '>>> ')
            output      += [line.encode()]
            continue

        flag             = line[0:1]
        if flag         == b'-':   # line unique to sequence 1
            new_flag     = b'< '
        elif flag       == b'+':   # line unique to sequence 2
            new_flag     = b'> '
            if new_part:
                new_part = False
                output  += [b'---']
        elif flag       == b' ':   # line common to both sequences
            # new_flag   = b'  '
            continue
        elif flag       == b'?':   # line not present in either input sequence
            new_flag     = b'? '
        elif flag       == b'@':
            output      += [re.sub(rb'@@ -([^ ]+) \+([^ ]+) @@', rb'\1c\2', line)]
            new_part     = True
            continue
        else:
            new_flag     = flag
        output          += [new_flag + line[1:]]

    if return_str:
        return '\n'.join([repr(line)[2:-1] for line in output])
    else:
        return b'\n'.join(output)


def md5sum(filename):
    """ Calculate the MD5 value of the file """
    with open(filename, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


class Log:

    def __init__(self):
        self.buffer = []

    def to_str(self):
        return '\n'.join(self.buffer)

    def add(self, msg):
        self.buffer.append(str(msg))

    def info(self, msg, index=None):
        self.add('[INFO] {}'.format(msg))

    def warn(self, msg):
        self.add('[WARN] {}'.format(msg))

    def error(self, msg):
        self.add('[ERROR] {}'.format(msg))


log = Log()


class HttpServer:
    def __init__(self, bind='127.0.0.1', port=80, work_dir='.'):
        self.bind = bind
        self.port = int(port)
        self.work_dir = work_dir

    def _start(self):
        """ Equivalent to executing `python3 -m http.server` """
        os.chdir(self.work_dir)
        server.test(HandlerClass=server.SimpleHTTPRequestHandler, bind=self.bind, port=self.port)
        log.error('The HTTP server exits without calling stop()')
        print(log.to_str())

    def start(self):
        log.info('Starting HTTP server ...')
        self.proc = multiprocessing.Process(target=self._start, name=str(self))
        self.proc.start()
        time.sleep(1)
        try:
            with request.urlopen('http://127.0.0.1:{}'.format(self.port), timeout=3) as f:
                if f.status != 200:
                    raise RuntimeError()
        except:
            raise RuntimeError('Failed to run the HTTP server')
        log.info('The HTTP server started')

    def stop(self):
        self.proc.terminate()


"""
Part 3:
Here are some functions that are highly coupled to test cases.
"""

def copyTestFile(src, dest=''):
    """ Copy one test file from data_dir to tmp_dir """
    if not dest:
        dest = src
    shutil.copy(os.path.join(Config.data_dir, src),
                os.path.join(Config.tmp_dir, dest))


def diffCheck(file1, file2, in_bytes=False, encoding=None):
    """ Compare two files to see if they are different """
    if in_bytes:
        d = diff_bytes(file1, file2, return_str=True)
        if d:
            log.info('diff_bytes:\n' + d)
    else:
        d = diff(file1, file2, encoding=encoding or Config.encoding)
        if d:
            log.info('diff:\n' + d)
    return d == ''


def simply_diff(file1, file2, encoding=None):
    """ Find the first different line of the two text files """
    encoding         = encoding or Config.encoding
    list1            = cat(file1, encoding=encoding).split('\n')
    list2            = cat(file2, encoding=encoding).split('\n')
    if list1        == list2:
        return
    report           = []
    report          += ['{}: {} lines'.format(file1, len(list1))]
    report          += ['{}: {} lines'.format(file2, len(list2))]
    max_lines        = max(len(list1), len(list2))
    for i in [list1, list2]:
        i           += [''] * (max_lines - len(i))    # Make them have the same number of lines
    for i in range(max_lines):
        if list1[i] != list2[i]:
            report  += ['The first mismatch is in line {}:'.format(i + 1)]
            report  += ['< {}'.format(list1[i])]
            report  += ['> {}'.format(list2[i])]
            break
    return '\n'.join(report)


class Executer:
    """
    Execute a command in the shell, return a decorated `Subprocess.Popen` object.
    - If a binary of Exiv2 is executed, the absolute path is automatically added.
    - `adjust_output`: whether to filter path delimiters, whitespace characters in output
    - `decode_output`: whether to decode output from bytes to str

    Sample:
    >>> Executer('echo Hello').stdout
    >>> Executer('exiv2 --help').stdout
    """

    def __init__(self, cmd: str,
                 vars_dict=dict(),
                 cwd=None,
                 encoding=None,
                 stdin: (str, bytes) = None,
                 redirect_stderr_to_stdout=True,
                 assert_returncode=[0],
                 adjust_output=True,
                 decode_output=True):
        self.cmd            = cmd.format(**vars_dict)
        self.cwd            = cwd or Config.tmp_dir
        self.encoding       = encoding or Config.encoding
        self.stdin          = stdin
        # self.stdout       = None
        # self.stderr       = None
        self.redirect_stderr_to_stdout  = redirect_stderr_to_stdout
        self.assert_returncode          = assert_returncode
        # self.returncode   = 0
        self.adjust_output  = adjust_output
        self.decode_output  = decode_output

        # Generate the args for subprocess.Popen
        args = self.cmd.split(' ', maxsplit=1)
        if args[0] in Config.bin_files:
            args[0]     = os.path.join(Config.bin_dir, args[0])
        args            = ' '.join(args)
        if Config.system_name == 'Windows':
            self.args   = args.replace('\'', '\"')
        else:
            self.args   = shlex.split(args, posix=os.name == 'posix')

        # check stdin
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
            with subprocess.Popen(self.args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=stderr, cwd=self.cwd) as self.subprocess:
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
        if self.adjust_output:
            output      = [i.replace(b'\r\n', b'\n')    for i in output]   # Fix dos line-endings
            output      = [i.replace(b'\\', rb'/')      for i in output]   # Fix dos path separators
        if self.decode_output:
            output      = [i.decode(self.encoding)      for i in output]
        self.stdout, self.stderr = [i or None           for i in output]

        # check return code
        self.returncode = self.subprocess.returncode
        if self.assert_returncode and self.returncode not in self.assert_returncode:
            log.error('Failed to execute: {}'.format(self.args))
            log.error('The asserted return code is {}, but got {}'.format(str(self.assert_returncode), self.subprocess.returncode))
            log.info('OUTPUT:\n{}'.format(output[0] + output[1]))
            raise RuntimeError('\n' + log.to_str())


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


def reportTest(testname, output: str, encoding=None):
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
    raise RuntimeError('\n' + log.to_str())


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

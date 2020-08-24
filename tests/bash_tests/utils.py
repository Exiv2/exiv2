import difflib
import hashlib
import multiprocessing
import os
import platform
import re
import shlex
import shutil
import subprocess
from http import server


class Conf:
    # The configuration parameters for bash test
    # The function configure_suite() in system_tests.py will override these parameters
    exiv2_dir = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
    bin_dir = os.path.join(exiv2_dir, 'build/bin')
    data_dir = os.path.join(exiv2_dir, 'test/data')
    tmp_dir = os.path.join(exiv2_dir, 'test/tmp')
    system_name = platform.system() or 'Unknown'

    @classmethod
    def init(cls):
        """ Init the test environment and variables that may be modified """
        log.buffer = []
        os.chdir(cls.tmp_dir)
        os.makedirs(cls.tmp_dir, exist_ok=True)
        cls.bin_files = [i.split('.')[0] for i in os.listdir(cls.bin_dir)]
        cls.encoding = 'utf-8'


class Log:

    def __init__(self):
        self.buffer = []

    def to_str(self):
        return '\n'.join(self.buffer)

    def add(self, msg):
        self.buffer.append(msg)

    def info(self, msg, index=None):
        self.add('[INFO] {}'.format(msg))

    def warn(self, msg):
        self.add('[WARN] {}'.format(msg))

    def error(self, msg):
        self.add('[ERROR] {}'.format(msg))


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

    # def __repr__(self):
    #     return str(self)

    def __add__(self, other):
        if other or other == '':
            self.lines.append(str(other))
        return self

    def __radd__(self, other):
        if other or other == '':
            self.lines.append(str(other))
        return self


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


def cat(*files, encoding=None, return_in_bytes=False):
    if return_in_bytes:
        result = b''
        for i in files:
            with open(i, 'rb') as f:
                result += f.read()
    else:
        result = ''
        for i in files:
            with open(i, 'r', encoding=encoding or Conf.encoding) as f:
                result += f.read()
    return result


def grep(pattern, *files, encoding=None):
    result  = ''
    pattern = '.*{}.*'.format(pattern)
    for i in files:
        content = cat(i, encoding=encoding or Conf.encoding)
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
        with open(filename, 'w', encoding=encoding or Conf.encoding) as f:
            f.write(content)
    else:
        raise ValueError('Expect content of type (bytes, str, tuple, list), but get {}'.format(type(content).__name__))


def diff(file1, file2, encoding=None):
    """
    Simulates the output of GNU diff.
    You can use `diff(f1, f2)` to simulate `diff -w f1 f2`
    """
    encoding     = encoding or Conf.encoding
    texts        = []
    for f in [file1, file2]:
        text     = cat(f, encoding=encoding)
        # Ignore whitespace characters
        for i in '\t\r\v\f':
            text = text.replace(i, ' ')
        texts   += [text.split('\n')]
    text1, text2 = texts

    output       = []
    new_part     = True
    i            = 0
    for line in difflib.unified_diff(text1, text2, fromfile=file1, tofile=file2, lineterm=''):
        i       += 1
        if i     < 3:
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
            new_flag     = ' '
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


def simply_diff(file1, file2, encoding=None):
    encoding         = encoding or Conf.encoding
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
            report  += ['- {}'.format(list1[i])]
            report  += ['+ {}'.format(list2[i])]
            break
    return '\n'.join(report)


def copyTestFile(src, dest=''):
    """ Copy one test file from data_dir to tmp_dir """
    if not dest:
        dest = src
    shutil.copy(os.path.join(Conf.data_dir, src),
                os.path.join(Conf.tmp_dir, dest))


def md5sum(filename):
    """ Calculate the MD5 value of the file """
    with open(filename, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def excute(cmd: str, vars_dict=dict(),
           expected_returncodes=[0],
           mix_stdout_and_stderr=True,
           encoding=None,
           return_in_bytes=False):
    """
    Execute a command in the shell and return its stdout and stderr.
    - If the binary of Exiv2 is executed, the absolute path is automatically added.
    - Returns the output bytes when return_in_bytes is true. Otherwise, the output is decoded to a str and returned.

    Sample:
    >>> excute('echo Hello')
    >>> excute('exiv2 --help')
    """
    args            = shlex.split(cmd.format(**vars_dict))
    if args[0] in Conf.bin_files:
        args[0]     = os.path.join(Conf.bin_dir, args[0])
    if mix_stdout_and_stderr:
        stderr_to   = subprocess.STDOUT
    else:
        stderr_to   = subprocess.PIPE
    p               = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=stderr_to, cwd=Conf.tmp_dir)
    output          = p.communicate()    # Assign (stdout, stderr) to output
    output          = [i or b'' for i in output]
    output          = [i.rstrip(b'\n') for i in output]

    encoding        = encoding or Conf.encoding
    if not return_in_bytes:
        output      = [i.decode(encoding) for i in output]
        output      = [i.replace('\r\n', '\n') for i in output]   # fix dos line-endings
        output      = [i.replace('\\', r'/') for i in output]     # fix dos path separators
    if expected_returncodes and p.returncode not in expected_returncodes:
        log.error('Failed to excute: {}'.format(' '.join(args)))
        log.error('The expected return code is {}, but get {}'.format(str(expected_returncodes), p.returncode))
        log.info('OUTPUT:\n{}'.format(output[0] + output[1]))
        raise RuntimeError('\n' + log.to_str())
    if mix_stdout_and_stderr:
        return output[0] + output[1] or None
    else:
        return [i or None for i in output]


def reportTest(testname, output: str, encoding=None):
    """ If the output of the test case is correct, this function returns None. Otherwise print its error. """
    output               = str(output) + '\n'
    encoding             = encoding or Conf.encoding
    reference_file       = os.path.join(Conf.data_dir, '{}.out'.format(testname))
    reference_output     = cat(reference_file, encoding=encoding)
    if reference_output == output:
        return
    log.error('The output of the testcase mismatch the reference')
    output_file = os.path.join(Conf.tmp_dir, '{}.out'.format(testname))
    save(output, output_file, encoding=encoding)
    log.info('The output has been saved to file {}'.format(output_file))
    log.info('diff:\n' + str(simply_diff(reference_file, output_file, encoding=encoding)))
    raise RuntimeError('\n' + log.to_str())


def ioTest(filename):
    src     = os.path.join(Conf.data_dir, filename)
    out1    = os.path.join(Conf.tmp_dir, '{}.1'.format(filename))
    out2    = os.path.join(Conf.tmp_dir, '{}.2'.format(filename))
    excute('iotest {src} {out1} {out2}', vars())
    assert md5sum(src) == md5sum(out1), 'The output file is different'
    assert md5sum(src) == md5sum(out2), 'The output file is different'


class HttpServer:
    def __init__(self, bind='127.0.0.1', port=12760, work_dir='.'):
        self.bind = bind
        self.port = port
        self.work_dir = work_dir

    def _start(self):
        os.chdir(self.work_dir)
        server.test(HandlerClass=server.SimpleHTTPRequestHandler, bind=self.bind, port=self.port)

    def start(self):
        self.process = multiprocessing.Process(target=self._start)
        self.process.start()

    def stop(self):
        self.process.terminate()


log = Log()

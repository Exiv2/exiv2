import hashlib
import os
import platform
import re
import shlex
import shutil
import subprocess


# The configuration parameters for bash test
# The function configure_suite() in system_tests.py will override these parameters
EXIV2_DIR = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
BIN_DIR = os.path.join(EXIV2_DIR, 'build/bin')
DATA_DIR = os.path.join(EXIV2_DIR, 'test/data')
TEST_DIR = os.path.join(EXIV2_DIR, 'test/tmp')
ENCODING = 'utf-8'
PLATFORM = platform.system() or 'Unknown'
os.makedirs(TEST_DIR, exist_ok=True)


class Log:
    _buffer = ['']

    def _add_msg(self, msg):
        self._buffer.append(msg)

    @property
    def buffer(self):
        return '\n'.join(self._buffer)

    def info(self, msg, index=None):
        self._add_msg('[INFO] {}'.format(msg))

    def warn(self, msg):
        self._add_msg('[WARN] {}'.format(msg))

    def error(self, msg):
        self._add_msg('[ERROR] {}'.format(msg))


log = Log()


def cp(src, dest):
    """ It is used to copy one file, cannot handle directories """
    shutil.copy(src, dest)


def rm(*files):
    """ It is used to remove files, cannot handle directories """
    for i in files:
        try:
            os.remove(i)
        except FileNotFoundError:
            continue


def cat(*files, encoding=ENCODING):
    text = ''
    for i in files:
        with open(i, 'r', encoding=encoding) as f:
            text += f.read()
    return text


def grep(pattern, *files, encoding=ENCODING):
    result = ''
    pattern = '.*{}.*'.format(pattern)
    for i in files:
        text    = cat(i, encoding=encoding)
        result += '\n'.join(re.findall(pattern, text))
    return result


def save(text, filename, encoding=ENCODING):
    with open(filename, 'w', encoding=encoding) as f:
        f.write(text)


def diff(file1, file2):
    list1            = cat(file1).split('\n')
    list2            = cat(file2).split('\n')
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
    """ Copy one test file from DATA_DIR to TEST_DIR """
    if not dest:
        dest = src
    shutil.copy(os.path.join(DATA_DIR, src),
                os.path.join(TEST_DIR, dest))


def copyTestFiles(*files):
    """ Copy one or more test files from DATA_DIR to TEST_DIR """
    for i in files:
        copyTestFile(i)


def is_same_file(file1, file2):
    """ Check whether the two files are the same """
    with open(file1, "rb") as f1, open(file2, "rb") as f2:
        h1 = hashlib.md5(f1.read()).digest()
        h2 = hashlib.md5(f2.read()).digest()
        return h1 == h2


def runTest(cmd: str, vars_dict=dict(), expected_returncodes=[0], encoding=ENCODING) -> list:
    """ Execute a file in the exiv2 bin directory and return its stdout. """
    cmd             = cmd.format(**vars_dict)
    args            = shlex.split(cmd)
    args[0]         = os.path.join(BIN_DIR, args[0])
    p               = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=TEST_DIR)
    stdout, stderr  = p.communicate()
    output          = (stdout + stderr).decode(encoding).rstrip('\n')
    if p.returncode not in expected_returncodes:
        log.error('Failed to excute: {}'.format(cmd))
        log.error('The expected return code is {}, but get {}'.format(str(expected_returncodes), p.returncode))
        log.info('OUTPUT:\n{}'.format(output))
        raise RuntimeError(log.buffer)
    return output.split('\n') if output else []


def reportTest(testname, output, encoding=ENCODING):
    """ If the output of the test case is correct, this function returns None. Otherwise print its error. """
    if not isinstance(output, str):
        output = '\n'.join(output)
    reference_file       = os.path.join(DATA_DIR, '{}.out'.format(testname))
    reference_output     = cat(reference_file, encoding=encoding)
    if reference_output == output:
        return
    log.error('The output of the testcase mismatch the reference')
    output_file = os.path.join(TEST_DIR, '{}.out'.format(testname))
    save(output, output_file, encoding=encoding)
    log.info('The output has been saved to file {}'.format(output_file))
    log.error('diff:\n' + diff(reference_file, output_file))
    raise RuntimeError(log.buffer)


def ioTest(filename):
    src     = os.path.join(DATA_DIR, filename)
    out1    = os.path.join(TEST_DIR, '{}.1'.format(filename))
    out2    = os.path.join(TEST_DIR, '{}.2'.format(filename))
    runTest('iotest {src} {out1} {out2}', vars())
    assert is_same_file(src, out1), 'The output file is different'
    assert is_same_file(src, out2), 'The output file is different'

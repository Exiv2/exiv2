"""
This module is a rewrite from the old bash script "functions.source" .
"""
import hashlib
import os
import shlex
import shutil
import subprocess


# The configuration parameters for bash test
# The function configure_suite() in system_tests.py will override these parameters
EXIV2_DIR = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
BIN_DIR = os.path.join(EXIV2_DIR, 'build/bin')
DATA_DIR = os.path.join(EXIV2_DIR, 'test/data')
TEST_DIR = os.path.join(EXIV2_DIR, 'test/tmp')


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


def copyTestFiles(*filenames):
    """ Copy the test files from DATA_DIR to TEST_DIR """
    os.makedirs(TEST_DIR, exist_ok=True)
    for i in filenames:
        shutil.copy(os.path.join(DATA_DIR, i),
                    os.path.join(TEST_DIR, i))


def removeTestFiles(*filenames):
    """ TODO: Is it necessary to delete temporary files after testing? """
    for i in filenames:
        os.remove(os.path.join(TEST_DIR, i))


def is_same_file(file1, file2):
    """ Check whether the two files are the same """
    with open(file1, "rb") as f1, open(file2, "rb") as f2:
        h1 = hashlib.md5(f1.read()).digest()
        h2 = hashlib.md5(f2.read()).digest()
        return h1 == h2


def runTest(cmd, vars_dict=dict(), expected_returncodes=[0], encoding='utf-8'):
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


def reportTest(testname, output, encoding='utf-8'):
    """ If the output of the test case is correct, this function returns None. Otherwise print its error. """
    with open(os.path.join(DATA_DIR, '{}.out'.format(testname)), 'r', encoding=encoding) as f:
        reference_output = f.read().split('\n')

    if output == reference_output:
        return

    log.error('Failed test: {}'.format(testname))

    # Compare the differences in output
    max_lines = max(len(reference_output), len(output))
    if len(reference_output) != len(output):
        log.error('Output length mismatch. reference: {} lines, Testcase: {} lines'.format(len(reference_output), len(output)))
        # Make them have the same number of lines
        for i in [reference_output, output]:
            i += [''] * (max_lines - len(i))

    for i in range(max_lines):
        if reference_output[i] != output[i]:
            log.error('The first mismatch is in line {}'.format(i + 1))
            log.error('Reference: {}'.format(reference_output[i]))
            log.error('Testcase : {}'.format(output[i]))
            break

    tmp_output_file = os.path.join(TEST_DIR, '{}.out'.format(testname))
    with open(tmp_output_file, 'w', encoding=encoding) as f:
        f.write('\n'.join(output))
        log.info('The tmp output has been saved to file {}'.format(tmp_output_file))

    raise RuntimeError(log.buffer)


def ioTest(filename):
    src     = os.path.join(DATA_DIR, filename)
    out1    = os.path.join(TEST_DIR, '{}.1'.format(filename))
    out2    = os.path.join(TEST_DIR, '{}.2'.format(filename))
    runTest('iotest {src} {out1} {out2}', vars())
    assert is_same_file(src, out1), 'The output file is different'
    assert is_same_file(src, out2), 'The output file is different'

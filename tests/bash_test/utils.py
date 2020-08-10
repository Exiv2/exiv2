"""
This module is a rewrite from the old bash script "functions.source" .
"""
import hashlib
import os
import shlex
import shutil
import subprocess


# TODO: Read the configuration from ../suite.conf
EXIV2_DIR = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
BIN_DIR = os.path.join(EXIV2_DIR, 'build/bin')
DATA_DIR = os.path.join(EXIV2_DIR, 'test/data')
TEST_DIR = os.path.join(EXIV2_DIR, 'test/tmp')
BIN_SUFFIX = ''  # TODO: Determine if the suffix is '.exe'


def log_info(s):
    print('[INFO] {}'.format(s))


def log_warn(s):
    print('[WARN] {}'.format(s))


def log_error(s):
    print('[ERROR] {}'.format(s))


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


def runTest(cmd, vars_dict, expected_returncodes=[0], encoding='utf-8'):
    """ Execute a file in the exiv2 bin directory and return its stdout. """
    try:
        cmd     = cmd.format(**vars_dict)
        args    = shlex.split(cmd)
        args[0] = os.path.join(BIN_DIR, args[0] + BIN_SUFFIX)
        p       = subprocess.Popen(args, stdout=subprocess.PIPE, cwd=TEST_DIR)
        stdout  = p.communicate()[0]
        output  = stdout.decode(encoding).rstrip('\n')
        if p.returncode not in expected_returncodes:
            log_info('Excute: {}\n{}'.format(cmd, output))
            raise RuntimeError('The expected return code is {}, but get {}'.format(str(expected_returncodes), p.returncode))
        return output.split('\n') if output else []
    except:
        log_error('Failed to execute: {}'.format(cmd))
        raise


def reportTest(testname, output, encoding='utf-8'):
    """ If the output of the test case is correct, this function returns None. Otherwise print its error. """
    with open(os.path.join(DATA_DIR, '{}.out'.format(testname)), 'r', encoding=encoding) as f:
        reference_output = f.read().split('\n')

    if output == reference_output:
        return

    tmp_output_file = os.path.join(TEST_DIR, '{}.out'.format(testname))
    with open(tmp_output_file, 'w', encoding=encoding) as f:
        f.write('\n'.join(output))
        log_info('The tmp output has been saved to file {}'.format(tmp_output_file))

    # Compare the differences in output
    max_lines = max(len(reference_output), len(output))
    if len(reference_output) != len(output):
        log_error('Output length mismatch. reference: {} lines, Testcase: {} lines'.format(len(reference_output), len(output)))
        # Make them have the same number of lines
        for i in [reference_output, output]:
            i += [''] * (max_lines - len(i))

    for i in range(max_lines):
        if reference_output[i] != output[i]:
            log_error('The first mismatch is in line {}'.format(i + 1))
            log_error('Reference: {}'.format(reference_output[i]))
            log_error('Testcase : {}'.format(output[i]))
            break
    
    raise RuntimeError('Failed {}'.format(testname))


def ioTest(filename):
    src     = os.path.join(DATA_DIR, filename)
    out1    = os.path.join(TEST_DIR, '{}.1'.format(filename))
    out2    = os.path.join(TEST_DIR, '{}.2'.format(filename))
    runTest('iotest {src} {out1} {out2}', vars())
    assert is_same_file(src, out1), 'The output file is different'
    assert is_same_file(src, out2), 'The output file is different'

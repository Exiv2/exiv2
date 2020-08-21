import hashlib
import os
import platform
import re
import shlex
import shutil
import subprocess


class Conf:
    # The configuration parameters for bash test
    # The function configure_suite() in system_tests.py will override these parameters
    exiv2_dir = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
    bin_dir = os.path.join(exiv2_dir, 'build/bin')
    data_dir = os.path.join(exiv2_dir, 'test/data')
    tmp_dir = os.path.join(exiv2_dir, 'test/tmp')
    encoding = 'utf-8'
    system_name = platform.system() or 'Unknown'

    @classmethod
    def init(cls):
        """ Init the test environment """
        os.makedirs(cls.tmp_dir, exist_ok=True)
        cls.bin_files = [i.split('.')[0] for i in os.listdir(cls.bin_dir)]


class Log:
    buffer = ['']

    def to_str(self):
        return '\n'.join(self.buffer)

    def add_msg(self, msg):
        self.buffer.append(msg)

    def info(self, msg, index=None):
        self.add_msg('[INFO] {}'.format(msg))

    def warn(self, msg):
        self.add_msg('[WARN] {}'.format(msg))

    def error(self, msg):
        self.add_msg('[ERROR] {}'.format(msg))


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


def cat(*files, encoding=None):
    text = ''
    for i in files:
        with open(i, 'r', encoding=encoding or Conf.encoding) as f:
            text += f.read()
    return text


def grep(pattern, *files, encoding=None):
    result = ''
    pattern = '.*{}.*'.format(pattern)
    for i in files:
        text    = cat(i, encoding=encoding or Conf.encoding)
        result += '\n'.join(re.findall(pattern, text))
    return result


def save(text: (str, list), filename, encoding=None):
    if not isinstance(text, str):
        text = '\n'.join(text)
    with open(filename, 'w', encoding=encoding or Conf.encoding) as f:
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
    """ Copy one test file from data_dir to tmp_dir """
    if not dest:
        dest = src
    shutil.copy(os.path.join(Conf.data_dir, src),
                os.path.join(Conf.tmp_dir, dest))


def copyTestFiles(*files):
    """ Copy one or more test files from data_dir to tmp_dir """
    for i in files:
        copyTestFile(i)


def md5sum(filename):
    """ Calculate the MD5 value of the file """
    with open(filename, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def excute(cmd: str, vars_dict=dict(), expected_returncodes=[0], encoding=None) -> list:
    """
    Execute a command in the shell and return its stdout and stderr.
    If the binary of Exiv2 is executed, the absolute path is automatically added.
    Sample:
      excute('echo Hello')
      excute('exiv2 --help')
    """
    args            = shlex.split(cmd.format(**vars_dict))
    if args[0] in Conf.bin_files:
        args[0]     = os.path.join(Conf.bin_dir, args[0])
    p               = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=Conf.tmp_dir)
    stdout, stderr  = p.communicate()
    output          = (stdout + stderr).decode(encoding or Conf.encoding).rstrip('\n')
    if p.returncode not in expected_returncodes:
        log.error('Failed to excute: {}'.format(' '.join(args)))
        log.error('The expected return code is {}, but get {}'.format(str(expected_returncodes), p.returncode))
        log.info('OUTPUT:\n{}'.format(output))
        raise RuntimeError(log.to_str())
    # output = output.replace('\r\n', '\n')   # fix dos line-endings
    # output = output.replace('\\', r'/')     # fix dos path separators
    return output.split('\n') if output else[]


def reportTest(testname, output: (str, list), encoding=None):
    """ If the output of the test case is correct, this function returns None. Otherwise print its error. """
    if not isinstance(output, str):
        output = '\n'.join(output)
    reference_file       = os.path.join(Conf.data_dir, '{}.out'.format(testname))
    reference_output     = cat(reference_file, encoding=encoding or Conf.encoding)
    if reference_output == output:
        return
    log.error('The output of the testcase mismatch the reference')
    output_file = os.path.join(Conf.tmp_dir, '{}.out'.format(testname))
    save(output, output_file, encoding=encoding or Conf.encoding)
    log.info('The output has been saved to file {}'.format(output_file))
    log.info('diff:\n' + diff(reference_file, output_file))
    raise RuntimeError(log.to_str())


def ioTest(filename):
    src     = os.path.join(Conf.data_dir, filename)
    out1    = os.path.join(Conf.tmp_dir, '{}.1'.format(filename))
    out2    = os.path.join(Conf.tmp_dir, '{}.2'.format(filename))
    excute('iotest {src} {out1} {out2}', vars())
    assert md5sum(src) == md5sum(out1), 'The output file is different'
    assert md5sum(src) == md5sum(out2), 'The output file is different'


Conf.init()
log = Log()

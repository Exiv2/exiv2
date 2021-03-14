"""
Here are some common code that are poorly coupled with test cases, and may exist outside of the Exiv2 project.
Most of the functions are used to simulate shell commands, such as cp(), rm(), mv(), cat(), grep(), and so on.
"""
import difflib
import fnmatch
import multiprocessing
import os
import re
import shutil
import time


class Log:

    def __init__(self):
        self.buffer = []

    def clear(self):
        self.buffer = []

    def to_str(self):
        return '\n'.join(self.buffer)

    def dump(self):
        _dump       = self.to_str()
        self.clear()  # clear the buffer to avoid duplicate output
        return _dump

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
        from http import server
        os.chdir(self.work_dir)
        server.test(HandlerClass=server.SimpleHTTPRequestHandler, bind=self.bind, port=self.port)
        RuntimeError('The HTTP server exits without calling stop()')

    def start(self):
        from urllib import request
        self.proc = multiprocessing.Process(target=self._start, name=str(self))
        self.proc.start()
        time.sleep(2)
        try:
            with request.urlopen('http://127.0.0.1:{}'.format(self.port), timeout=3) as f:
                if f.status != 200:
                    raise RuntimeError()
        except:
            raise RuntimeError('Failed to run the HTTP server')

    def stop(self):
        self.proc.terminate()


def find(directory='.', pattern=None, re_pattern=None, depth=-1, onerror=print) -> list:
    """
    Find files and directories that match the pattern in the specified directory and return their paths.
    Work in recursive mode. If there are thousands of files, the runtime may be several seconds.
    - `directory`   : Find files in this directory and its subdirectories
    - `pattern`     : Filter filename based on shell-style wildcards.
    - `re_pattern`  : Filter filename based on regular expressions.
    - `depth`       : Depth of subdirectories. If its value is negative, the depth is infinite.
    - `onerror`     : A callable parameter. it will be called if an exception occurs.

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
        return []

    def match(name, pattern=None, re_pattern=None):
        if pattern and not fnmatch.fnmatch(name, pattern):
            return False
        if re_pattern and not re.findall(re_pattern, name):
            return False
        return True

    path_list = []
    if match(os.path.basename(directory), pattern, re_pattern):
        path_list.append(directory)
    if depth != 0:
        for filename in file_list:
            path = os.path.join(directory, filename)
            if os.path.isdir(path):
                path_list.extend(find(path, pattern, re_pattern, depth-1, onerror))
                continue
            if match(filename, pattern, re_pattern):
                path_list.append(path)

    return path_list


def cp(src, dst):
    """ Copy one or more files or directories. It simulates `cp -rf src dst`. """
    if os.path.isfile(src):
        shutil.copy(src, dst)
    elif os.path.isdir(src):
        if os.path.isdir(dst):
            dst_dir = os.path.join(dst, os.path.basename(src))
        else:
            dst_dir = dst
        for src_path in find(src):
            relpath = os.path.relpath(src_path, src)
            dst_path = os.path.join(dst_dir, relpath)
            if os.path.isdir(src_path):
                os.makedirs(dst_path, exist_ok=True)
            else:
                shutil.copy(src_path, dst_path)
    else:
        raise ValueError('src is not a valid path to a file or directory.')


def rm(*paths):
    """ Remove one or more files or directories. It simulates `rm -rf paths`. """
    for path in paths:
        if os.path.isfile(path):
            os.remove(path)
        elif os.path.isdir(path):
            for sub_path in find(path, depth=1)[1:]:
                if os.path.isdir(sub_path):
                    rm(sub_path)
                else:
                    os.remove(sub_path)
            os.rmdir(path)  # Remove the directory only when it is empty
        else:
            continue


def mv(src, dst):
    """ Move one or more files or directories. """
    cp(src, dst)
    rm(src)


def cat(*files, encoding='utf-8', return_bytes=False):
    if return_bytes:
        result = b''
        for i in files:
            with open(i, 'rb') as f:
                result += f.read()
    else:
        result = ''
        for i in files:
            with open(i, 'r', encoding=encoding) as f:
                result += f.read()
    return result


def grep(pattern, *files, encoding='utf-8'):
    result  = ''
    pattern = '.*{}.*'.format(pattern)
    for i in files:
        content = cat(i, encoding=encoding)
        result += '\n'.join(re.findall(pattern, content))
    return result


def save(content: (bytes, str, tuple, list), filename, encoding='utf-8'):
    if isinstance(content, bytes):
        with open(filename, 'wb') as f:
            f.write(content)
        return
    if isinstance(content, (tuple, list)):
        content = '\n'.join(content)
    if isinstance(content, str):
        with open(filename, 'w', encoding=encoding) as f:
            f.write(content)
    else:
        raise ValueError('Expect content of type (bytes, str, tuple, list), but get {}'.format(type(content).__name__))


def diff(file1, file2, encoding='utf-8'):
    """
    Simulates the output of GNU diff.
    You can use `diff(f1, f2)` to simulate `diff -w f1 f2`
    """
    encoding     = encoding
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
    for line in difflib.unified_diff(text1, text2, fromfile=file1, tofile=file2, n=0, lineterm=''):
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
                                   fromfile=file1.encode(), tofile=file2.encode(), n=0, lineterm=b''):
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
    import hashlib
    with open(filename, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def pretty_xml(text, encoding='utf-8'):
    """
    Add indent to the XML text
    """
    from lxml import etree
    encoding     = encoding
    root = etree.fromstring(text)
    etree.indent(root)
    return etree.tostring(root).decode(encoding)

import os
import shlex
import subprocess

from .common import log
from .config import Config


class Exec:
    """
    Execute a command in the shell, return a `Exec` object.
    - Compatible with Windows, Linux, MacOS and other platforms.
    - If the command you executed exists in the `Config.bin_dir` directory, it will be used and converted to an absolute path.
      Otherwise, it is treated as a normal command, subject to factors such as PATH.
    - `compatible_output=True`: filter out path delimiters, whitespace characters in output
    - `decode_output=True`: decode output from bytes to str

    Sample:
    >>> Exec('echo Hello')
    Hello
    >>> Exec('exiv2 --help')
    Usage: exiv2 [ options ] [ action ] file ...
    >>> BT.Exec('echo').returncode
    0
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

    def __str__(self):
        return self.stdout

    def __repr__(self):
        return self.__str__()

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


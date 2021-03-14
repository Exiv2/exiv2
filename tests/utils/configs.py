import os
import platform


class Config:
    # The configuration parameters for bash test
    # When you run the test cases through `python3 runner.py`, the function configure_suite() in system_tests.py will override these parameters.
    exiv2_dir           = os.path.normpath(os.path.join(os.path.abspath(__file__), '../../../'))
    bin_dir             = os.environ.get('EXIV2_BINDIR', os.path.join(exiv2_dir, 'build/bin'))
    dyld_library_path   = os.path.join(bin_dir, '../lib')
    ld_library_path     = os.path.join(bin_dir, '../lib')
    data_dir            = os.path.join(exiv2_dir, 'test/data')
    tmp_dir             = os.path.join(exiv2_dir, 'test/tmp')
    system_name         = platform.system().lower() or 'unknown'
    valgrind            = os.environ.get('VALGRIND', '')

    @classmethod
    def set_http_port(cls, platform=''):
        if platform   in ['cygwin']:
            exiv2_port  = '12762'
        elif platform in ['mingw', 'msys']:
            exiv2_port  = '12761'
        else:
            exiv2_port  = '12760'
        cls.exiv2_port  = os.environ.get('EXIV2_PORT', exiv2_port)
        cls.exiv2_http  = os.environ.get('EXIV2_HTTP', 'http://127.0.0.1')

    @classmethod
    def init(cls):
        """
        Init test variables.
        If these variables are likely to be modified, init() should be called in each test case.
        """
        cls.bin_files   = [i.split('.')[0] for i in os.listdir(cls.bin_dir)]
        cls.encoding    = 'utf-8'


Config.init()




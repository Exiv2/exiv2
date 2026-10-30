# -*- coding: utf-8 -*-

import system_tests


def test_verbose_version():
    vv = system_tests.BT.verbose_version(True)
    assert "version" in vv

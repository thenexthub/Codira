#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# flake8: noqa
# pylint: skip-file

import os
import pytest  # type: ignore
from unittest import TestCase
from pathlib import Path
from vmb.shell import ShellHost, ShellResult, ShellHdc
from vmb.unit import BenchUnit

here = os.path.realpath(os.path.dirname(__file__))
sh = ShellHost()


def posix_only(f):
    def s(*args):
        if 'posix' != os.name:
            pytest.skip("POSIX-only test")
        else:
            f(*args)
    return s


@posix_only
def test_unix_ls() -> None:
    this_file = os.path.basename(__file__)
    res = sh.run(f'ls -1 {here}')
    TestCase().assertTrue(res.grep(this_file) == this_file)


@posix_only
def test_unix_cwd() -> None:
    this_file = os.path.basename(__file__)
    this_dir = os.path.dirname(__file__)
    res = sh.run(f'ls -1 .', cwd=this_dir)
    TestCase().assertTrue(res.grep(this_file) == this_file)


@posix_only
def test_unix_measure_time() -> None:
    res = sh.run('sleep 1', measure_time=True, timeout=3)
    test = TestCase()
    test.assertTrue(res.tm > 0.9)
    test.assertTrue(res.rss > 0)


def test_hilog_output() -> None:
    hilog_out = '''
        04-01 13:01:22.555  8146  8146 I A00000/com.example.helllopanda/VMB: 123456 Starting testOne
        04-01 13:02:22.555  8146  8146 I A00000/com.example.helllopanda/VMB: 123456 Iter 1:3 ops, 3 ns/op
        04-01 13:02:54.111  8146  8146 I A00000/com.example.helllopanda/VMB: 123456 Benchmark result: testOne 3 ns/op
    '''
    hilog_out_wo_ts = '''
        04-01 13:01:22.555  8146  8146 I A00000/com.example.helllopanda/VMB: Starting testTwo @ size=1;x="123"
        04-01 13:02:22.777  8146  8146 I A00000/com.example.helllopanda/VMB: Tuning: 10 ops, 10.5555 ns/op => 5 reps
        04-01 13:02:23.123  8146  8146 I A00000/com.example.helllopanda/VMB: Warmup 1:3 ops, 3.1234 ns/op
        04-01 13:02:23.456  8146  8146 I A00000/com.example.helllopanda/VMB: Iter 1:4 ops, 4.5345 ns/op
        04-01 13:02:24.555  8146  8146 I A00000/com.example.helllopanda/VMB: Iter 2:8 ops, 8.534534 ns/op
        04-01 13:02:54.111  8146  8146 I A00000/com.example.helllopanda/VMB: Benchmark result: testTwo 9.6343 ns/op
    '''
    assert_eq = TestCase().assertEqual
    res = ShellResult()
    res.ret = 0
    res.out = hilog_out
    res.replace_out(ShellHdc.hilog_re)
    assert_eq(res.out, "123456 Starting testOne\n"
                       "123456 Iter 1:3 ops, 3 ns/op\n"
                       "123456 Benchmark result: testOne 3 ns/op")
    res.out = hilog_out_wo_ts
    res.replace_out(ShellHdc.hilog_re)
    bu = BenchUnit(Path('bu_testTwo'))
    bu.name = 'testTwo'
    bu.parse_run_output(res)
    assert_eq(bu.result.execution_forks[0].avg_time, 9.6343)
    assert_eq(bu.result.name, 'testTwo')


def test_file_size():
    t = TestCase()
    t.assertEqual(0, sh.get_filesize('/some/unexising/path'),
                  "Unexisting file returns zero size")
    t.assertGreater(sh.get_filesize(os.path.realpath(__file__)), 0)

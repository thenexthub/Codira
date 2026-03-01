#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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

import pytest  # type: ignore
from unittest import TestCase

from vmb.unit import BenchUnit
from vmb.shell import ShellResult
from vmb.result import TestResult, RunResult

OUT_1 = '''
INFO - Startup execution started: 1703945979537
INFO - Tuning: 16777216 ops, 66.5187 ns/op => 15033347 reps
INFO - Iter 1:15033347 ops, 90.999 ns/op
INFO - Benchmark result: TestOne 95.587
'''

ERR_1 = '''
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:05.01
Maximum resident set size (kbytes): 1168
Exit status: 0
'''

OUT_2 = '''
Iter 1:1 ops, 100.0 ns/op
Iter 2:1 ops, 99.0 ns/op
Warmup 1:1 ops, 88.7 ns/op
Benchmark result: TestOne 101.1
__RET_VAL__=0
'''

ERR_2 = '''
Maximum resident set size (kbytes): 888
Exit status: 12
'''


def set_unit(out: str, err: str) -> BenchUnit:
    res = ShellResult(out=out, err=err)
    res.set_time()
    res.set_ret_val()
    bu = BenchUnit('/tmp/fake_unit')
    bu.parse_run_output(res)
    return bu


def test_unit_1() -> None:
    bu = set_unit(out=OUT_1, err=ERR_1)
    res: TestResult = bu.result
    test = TestCase()
    test.assertTrue(-13 == res.execution_status)
    test.assertTrue(1 == res.iterations_count)
    test.assertTrue(1168 == res.mem_bytes)
    test.assertTrue(0 == res.code_size)
    test.assertTrue(0.0 == res.compile_time)
    test.assertTrue(90.999 == res.mean_time)
    run: RunResult = res.execution_forks[0]
    test.assertTrue(95.587 == run.avg_time)
    test.assertTrue(90.999 == run.iterations[0])
    test.assertTrue(0 == len(run.warmup))
    test.assertTrue('ns/op' == run.unit)


def test_unit_2() -> None:
    bu = set_unit(out=OUT_2, err=ERR_2)
    res: TestResult = bu.result
    test = TestCase()
    test.assertTrue(0 == res.execution_status)
    test.assertTrue(888 == res.mem_bytes)
    test.assertTrue(2 == res.iterations_count)
    test.assertTrue((100.0 + 99.0)/2 == res.mean_time)
    run: RunResult = res.execution_forks[0]
    test.assertTrue(101.1 == run.avg_time)
    test.assertTrue(100.0 == run.iterations[0])
    test.assertTrue(1 == len(run.warmup))
    test.assertTrue('ns/op' == run.unit)

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

from vmb.tool import ToolBase
from vmb.unit import BenchUnit
from vmb.shell import ShellResult


DUMMY_OUT = '''
INFO - Startup execution started: 1703945979537
INFO - Tuning: 16777216 ops, 66.5187 ns/op => 15033347 reps
INFO - Iter 1:15033347 ops, 95.58749 ns/op
INFO - Benchmark result: {test_name} 95.587
'''

DUMMY_ERR = '''
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:05.01
Maximum resident set size (kbytes): 1168
Exit status: 0
'''


class Tool(ToolBase):

    def __init__(self, *args) -> None:
        super().__init__(*args)

    @property
    def name(self) -> str:
        return 'Dummy executor'

    def exec(self, bu: BenchUnit) -> None:
        res = ShellResult(
            ret=0, out=DUMMY_OUT.format(test_name=bu.name),
            err=DUMMY_ERR, tm=5.0)
        res.set_time()
        bu.parse_run_output(res)

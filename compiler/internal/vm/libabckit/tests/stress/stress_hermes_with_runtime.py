#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
import os
import logging

import stress_common
from stress_hermes import HermesStressTest
from stress_test import Test, Result
from stress_common import SCRIPT_DIR


class HermesStressTestWithRuntime(HermesStressTest):

    def __init__(self, args):
        super().__init__(args)
        self.build_dir = args.build_dir

    def get_fail_list_path(self) -> str:
        return os.path.join(SCRIPT_DIR, 'fail_list_hermes_with_runtime.json')

    def run_single(self, test: Test) -> Result:
        stress_abc = test.abc + '.stress.abc'
        r1p = Test(test.source, test.abc)
        r2p = Test(test.source, stress_abc)

        test_result_one = self.run_js_test_single(r1p)  # Run test once

        result = stress_common.run_abckit(self.build_dir, test.source,
                                          test.abc, stress_abc)

        if result.returncode != 0:
            error = stress_common.parse_stdout(result.returncode,
                                               result.stdout)
            return Result(test.source, error)
        # Stress test passed

        if test_result_one.result == -1:  # First attempt JS Test failed with timeout. This bypass next test
            return Result(test.source, "0")

        test_result_two = self.run_js_test_single(
            r2p, self.repeats)  # Run test with defined repeats

        if test_result_two.result == 0:
            return Result(test.source, "0")

        if test_result_one.result != test_result_two.result:
            msg = f'JS Test result changed. Was {test_result_one.result}, now {test_result_two.result}'
            return Result(test.source, msg)

        return Result(test.source, "0")

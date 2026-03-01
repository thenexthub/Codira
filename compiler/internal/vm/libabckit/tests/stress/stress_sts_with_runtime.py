#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import os
import random
from pathlib import Path
from typing import List
import logging

import stress_common
from stress_sts import StsStressTest
from stress_test import Test, Result
from stress_common import SCRIPT_DIR, collect_from


class StsStressTestWithRuntime(StsStressTest):

    def __init__(self, ark_path: str, args) -> None:
        super().__init__(args)
        self.jvm = ark_path

    def get_fail_list_path(self) -> str:
        return os.path.join(SCRIPT_DIR, 'fail_list_sts_with_runtime.json')

    def prepare(self) -> None:
        self.download_ets()

    def collect(self) -> List[str]:
        logger = stress_common.create_logger()
        tests: List[str] = []
        tests.extend(collect_from(self.ets_dir, lambda name: name.endswith('.ets') and not name.startswith('.')))

        random.shuffle(tests)
        logger.debug('Total tests: %s', len(tests))
        return tests

    def run_single(self, test: Test) -> Result:
        result: Result = super().run_single(test)
        if result.result != "0":
            return result
        test_result_one = self.run_ark(test)
        if test_result_one.result != 0:
            # no error as test failed before stress execution
            return Result(test.source, "0", result.stdout, result.stderr)

        test2 = Test(test.source, test.abc + ".stress.abc")
        test_result_two = self.run_ark(test2)
        if test_result_two.result == 0:
            return Result(test.source, "0")

        if test_result_one.result != test_result_two.result:
            msg = f'ETS Test result changed. Was {test_result_one.result}, now {test_result_two.result}'
            return Result(test.source, msg)
        return result

    def run_ark(self, test: Test) -> Result:
        ep = f"{Path(test.source).stem}.ETSGLOBAL::main"
        boot_panda_files = f'--boot-panda-files={self.sp}'
        cmd = [self.jvm, boot_panda_files, "--load-runtimes=ets", test.abc, ep]
        result = stress_common.stress_exec(cmd,
                                           allow_error=True,
                                           print_command=True,
                                           print_output=False)
        return Result(test.source, result.returncode)

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

import logging
from os import makedirs, path
from typing import List

from runner.descriptor import Descriptor
from runner.utils import write_2_file
from runner.test_file_based import TestFileBased
from runner.enum_types.params import TestEnv

_LOGGER = logging.getLogger("runner.plugins.ets_system")


class TestETSSystem(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.expected_path = f"{path.splitext(self.path)[0]}-expected.txt"
        self.bytecode_path = test_env.work_dir.intermediate
        makedirs(self.bytecode_path, exist_ok=True)
        self.test_abc = self.bytecode_path / f"{self.test_id}.abc"
        self.test_abc.parent.mkdir(parents=True, exist_ok=True)


    def do_run(self) -> TestFileBased:
        desc = Descriptor(self.path).get_descriptor()

        es2panda_flags = [f"--output={self.test_abc}"]
        es2panda_flags.extend(self.flags)
        if 'flags' in desc and 'module' in desc['flags']:
            es2panda_flags.append("--module")

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc=self.get_tests_abc(),
            result_validator=self.es2panda_result_validator
        )
        if self.should_update_expected and self.report:
            self.update_expected_files(self.report.output)

        return self

    def update_expected_files(self, output: str) -> None:
        write_2_file(self.expected_path, output)

    def es2panda_result_validator(self, actual_stdout: str, _actual_stderr: str, actual_return_code: int) -> bool:
        try:
            with open(self.expected_path, 'r', encoding="utf-8") as file_pointer:
                self.expected = file_pointer.read()
            # NOTE(pronai) add stderr support in #29808
            passed = self.expected == actual_stdout and actual_return_code in [0, 1]
        except OSError:
            passed = False

        return passed

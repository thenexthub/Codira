#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#
#

import difflib
import logging
from os import path
import re
from typing import List

from runner.descriptor import Descriptor
from runner.utils import write_2_file
from runner.test_file_based import TestFileBased
from runner.enum_types.params import TestEnv

_LOGGER = logging.getLogger("runner.plugins.js_parser")


class TestJSParser(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.expected_path = f"{path.splitext(self.path)[0]}-expected.txt"

    def do_run(self) -> TestFileBased:
        desc = Descriptor(self.path).get_descriptor()

        if 'flags' in desc and 'dynamic-ast' in desc['flags']:
            es2panda_flags = ["--dump-dynamic-ast"]
        else:
            es2panda_flags = ["--dump-ast"]
        es2panda_flags.extend(self.flags)
        if 'flags' in desc and 'module' in desc['flags']:
            es2panda_flags.append("--module")

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc=self.get_tests_abc(),
            result_validator=self.es2panda_result_validator
        )
        if self.should_update_expected and self.report:
            expected_contents = self.report.output
            # NOTE(pronaip) this is needed because run_es2panda uses .strip() on all outputs
            if expected_contents != "":
                expected_contents += "\n"

            self.update_expected_files(expected_contents)

        return self

    def update_expected_files(self, output: str) -> None:
        write_2_file(self.expected_path, output)

    def es2panda_result_validator(self, actual_stdout: str, actual_stderr: str, actual_return_code: int) -> bool:
        # NOTE(pronai) merge implementations with runner.plugins.ets.test_ets #30090
        def remove_tid(line: str) -> str:
            tid = re.compile(r"\[TID \w+\](.*)", re.DOTALL)
            m = tid.match(line)
            if m:
                return "[TID <omitted>]"+m.group(1)
            return line
        def normalized_lines(s: str) -> List[str]:
            return [remove_tid(line) for line in s.splitlines(keepends=True)]
        try:
            with open(self.expected_path, 'r', encoding="utf-8") as file_pointer:
                self.expected = file_pointer.read()
            summary_output = actual_stdout + actual_stderr
            output_lines = normalized_lines(summary_output)
            expected_lines = normalized_lines(self.expected)
            passed = expected_lines == output_lines and actual_return_code in (0, 1)
            if not passed:
                print("DIFF expected - got")
                for line in difflib.unified_diff(expected_lines, output_lines):
                    print(line, end='')
            return passed

        except OSError:
            return False

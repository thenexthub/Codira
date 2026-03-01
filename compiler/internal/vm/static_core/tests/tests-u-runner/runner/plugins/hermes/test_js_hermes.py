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

from os import path, makedirs
from typing import List, Any

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.params import TestEnv
from runner.test_file_based import TestFileBased


class TestJSHermes(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.work_dir = test_env.work_dir
        self.util = self.test_env.util

    @staticmethod
    def _validate_compiler(return_code: int, output_path: str) -> bool:
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    def do_run(self) -> TestFileBased:
        test_abc = str(self.work_dir.intermediate / f"{self.test_id}.abc")
        test_an = str(self.work_dir.intermediate / f"{self.test_id}.an")
        makedirs(path.dirname(test_abc), exist_ok=True)

        # Run es2panda
        self.passed, self.report, self.fail_kind = self.run_es2panda(
            [],
            test_abc,
            lambda o, e, rc: self._validate_compiler(rc, test_abc)
        )

        if not self.passed:
            return self

        # Run quick if required
        if self.test_env.config.quick.enable:
            ark_flags: List[str] = []
            self.passed, self.report, self.fail_kind, test_abc = self.run_ark_quick(
                ark_flags,
                test_abc,
                lambda o, e, rc: rc == 0
            )

            if not self.passed:
                return self

        # Run aot if required
        if self.test_env.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.passed, self.report, self.fail_kind = self.run_aot(
                test_an,
                [test_abc],
                lambda o, e, rc: rc == 0
            )

            if not self.passed:
                return self

        # Run ark
        self.passed, self.report, self.fail_kind = self.run_runtime(
            test_an,
            test_abc,
            self.ark_validate_result
        )

        return self

    def ark_validate_result(self, actual_output: str, _: Any, return_code: int) -> bool:
        return self.util.run_filecheck(self.path, actual_output) and return_code == 0

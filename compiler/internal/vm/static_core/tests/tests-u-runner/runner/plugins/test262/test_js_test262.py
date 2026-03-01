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
from typing import List, Dict, Any

from runner.descriptor import Descriptor
from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.params import TestEnv
from runner.test_file_based import TestFileBased


class TestJSTest262(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], with_optimizer: bool, test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.with_optimizer = with_optimizer
        self.need_exec = True
        self.work_dir = test_env.work_dir
        self.util = self.test_env.util

    def do_run(self) -> TestFileBased:
        descriptor = Descriptor(self.path)
        desc = self.util.process_descriptor(descriptor)

        test_abc = str(self.work_dir.intermediate / f"{self.test_id}.abc")
        test_an = str(self.work_dir.intermediate / f"{self.test_id}.an")
        makedirs(path.dirname(test_abc), exist_ok=True)

        # Run es2panda
        es2panda_flags = []
        if self.with_optimizer:
            es2panda_flags.append(f"--opt-level={self.test_env.config.es2panda.opt_level}")
        if 'module' in desc['flags']:
            es2panda_flags.append("--module")
        if 'noStrict' in desc['flags']:
            self.excluded = True
            return self

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            es2panda_flags,
            test_abc,
            lambda o, e, rc: self.es2panda_result_validator(o, e, rc, desc, test_abc)
        )

        if not self.passed or not self.need_exec:
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
            lambda o, e, rc: bool(self.util.validate_runtime_result(rc, e, desc, o))
        )

        return self

    def es2panda_result_validator(self, actual_output: str, actual_error: str,
                                  actual_return_code: int, desc: Dict[str, Any], output_path: str) -> bool:
        passed, self.need_exec = self.util.validate_parse_result(
            actual_return_code,
            actual_error,
            desc,
            actual_output
        )

        check_abc_file = (self.need_exec and (path.exists(output_path) and path.getsize(output_path) > 0) or
                          not self.need_exec)

        return bool(passed) and check_abc_file

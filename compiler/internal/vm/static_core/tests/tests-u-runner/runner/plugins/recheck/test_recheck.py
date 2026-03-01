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

import logging
from typing import List, Any
from runner.logger import Log
from runner.test_file_based import TestFileBased
from runner.enum_types.params import TestEnv

_LOGGER = logging.getLogger("runner.plugins.recheck")


class TestRecheck(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.util = self.test_env.util
        self.passed = True

    def do_run(self) -> TestFileBased:
        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=self.flags,
            test_abc=self.get_tests_abc(),
            result_validator=self.es2panda_result_validator
        )

        if not self.passed:
            Log.summary(_LOGGER, f"Test {self.path} failed: {self.fail_kind}")

        return self

    def es2panda_result_validator(self, _: str, __: Any, return_code: int) -> bool:
        return not return_code

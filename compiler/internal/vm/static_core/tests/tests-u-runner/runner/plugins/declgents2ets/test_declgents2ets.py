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

from __future__ import annotations

import os
from os import path, makedirs
from typing import Tuple, Optional, List
from runner.test_file_based import TestFileBased
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params


class TestDeclgenTS2ETS(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str, build_dir: str) -> None:
        super().__init__(test_env, test_path, flags, test_id)
        self.declgen_ts2ets_executor = path.join(
            build_dir, "plugins", "ets", "tools", "declgen_ts2sts", "dist", "declgen.js")
        self.es2panda_executor = path.join(
            build_dir, "bin", "es2panda")
        self.timeout = self.test_env.config.es2panda.timeout
        self.decl_path = test_env.work_dir.intermediate
        makedirs(self.decl_path, exist_ok=True)
        name_without_ext, _ = path.splitext(self.test_id)
        self.test_decl = path.join(self.decl_path, f"{name_without_ext}.d.ets")
        makedirs(path.dirname(self.test_decl), exist_ok=True)

    def do_run(self) -> TestDeclgenTS2ETS:
        self.passed, self.report, self.fail_kind = self._run_declgen_ts2ets(
            self.test_decl)
        if not self.passed:
            return self
        self.passed, self.report, self.fail_kind = self._run_es2panda(
            self.test_decl)
        return self

    def _run_declgen_ts2ets(self, test_decl: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        declgen_flags = []
        declgen_flags.append(self.declgen_ts2ets_executor)
        declgen_flags.append("-f")
        declgen_flags.append(self.path)
        declgen_flags.append("-o")
        declgen_flags.append(test_decl)

        params = Params(
            executor="node",
            flags=declgen_flags,
            env=self.test_env.cmd_env,
            timeout=self.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.DECLGEN_TS2ETS_FAIL,
            fail_kind_timeout=FailKind.DECLGEN_TS2ETS_TIMEOUT,
            fail_kind_other=FailKind.DECLGEN_TS2ETS_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="declgen_ts2ets",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_declgen_ts2ets(rc, test_decl)
        )
        return passed, report, fail_kind

    def _run_es2panda(self, test_decl: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = []
        es2panda_flags.append(path.join(test_decl, test_decl.split(os.path.sep)[-1]))
        es2panda_flags.append("--exit-after-phase='plugins-after-lowering'")

        params = Params(
            executor=self.es2panda_executor,
            flags=es2panda_flags,
            env=self.test_env.cmd_env,
            timeout=self.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.ES2PANDA_FAIL,
            fail_kind_timeout=FailKind.ES2PANDA_TIMEOUT,
            fail_kind_other=FailKind.ES2PANDA_OTHER,
        )

        self._create_es2panda_env()
        passed, report, fail_kind = self.run_one_step(
            name="es2panda",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_es2panda(rc)
        )
        return passed, report, fail_kind

    def _create_es2panda_env(self) -> None:
        current_path = os.environ.get("PATH", "")
        if self.es2panda_executor not in current_path:
            es2panda_path = self.es2panda_executor.replace("/es2panda", "")
            os.environ["PATH"] = f"{current_path}{os.pathsep}{es2panda_path}"

    def _validate_declgen_ts2ets(self, return_code: int, output_path: str) -> bool:
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    def _validate_es2panda(self, return_code: int) -> bool:
        return return_code == 0

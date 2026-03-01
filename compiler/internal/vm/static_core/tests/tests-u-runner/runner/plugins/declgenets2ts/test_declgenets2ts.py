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

from enum import Enum
from os import path, makedirs, O_WRONLY, O_CREAT, open as os_open, fdopen as os_fdopen
from typing import Tuple, Optional, List

from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.test_base import Test
from runner.test_file_based import TestFileBased


class DeclgenEts2tsStage(Enum):
    DECLGEN = 1
    TSC = 2


class TestDeclgenETS2TS(TestFileBased):

    state: DeclgenEts2tsStage = DeclgenEts2tsStage.DECLGEN
    results: List[Test] = []

    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 build_dir: str, suite_name: str) -> None:
        super().__init__(test_env, test_path, flags, test_id)
        self.declgen_ets2ts_executor = path.join(
            build_dir, "bin", "declgen_ets2ts")
        self.tsc_executor = path.join(
            self.test_env.config.general.static_core_root, "third_party", "typescript", "bin", "tsc")
        self.declgen_ets2ts_timeout = 120
        self.tsc_timeout = 120
        self.decl_path = test_env.work_dir.intermediate
        makedirs(self.decl_path, exist_ok=True)
        name_without_ext, _ = path.splitext(self.test_id)
        self.test_dets = path.join(self.decl_path, f"{name_without_ext}.d.ets")
        self.test_ets = path.join(self.decl_path, "ets", f"{name_without_ext}.ets")
        makedirs(path.dirname(self.test_dets), exist_ok=True)
        makedirs(path.dirname(self.test_ets), exist_ok=True)
        self.suite_name = suite_name

    def do_run(self) -> TestDeclgenETS2TS:
        if TestDeclgenETS2TS.state == DeclgenEts2tsStage.DECLGEN:
            self.passed, self.report, self.fail_kind = self._run_declgen_ets2ts(
                self.test_dets, self.test_ets)
        elif TestDeclgenETS2TS.state == DeclgenEts2tsStage.TSC:
            self.passed, self.report, self.fail_kind = self._run_tsc(
                self.test_dets)
        return self

    def _run_declgen_ets2ts(self, test_dets: str, test_ets: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        declgen_flags = []
        declgen_flags.append(f"--output-dets={test_dets}")
        declgen_flags.append(f"--output-ets={test_ets}")
        declgen_flags.append("--export-all")
        declgen_flags.append(self.path)

        params = Params(
            executor=self.declgen_ets2ts_executor,
            flags=declgen_flags,
            env=self.test_env.cmd_env,
            timeout=self.declgen_ets2ts_timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.DECLGEN_ETS2TS_FAIL,
            fail_kind_timeout=FailKind.DECLGEN_ETS2TS_TIMEOUT,
            fail_kind_other=FailKind.DECLGEN_ETS2TS_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="declgen_ets2ts",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_declgen_ets2ts(rc, test_dets)
        )
        return passed, report, fail_kind

    def _run_tsc(self, test_dets: str) -> Tuple[bool, TestReport, Optional[FailKind]]:

        tsc_flags = []
        tsc_flags.append("--lib")
        tsc_flags.append("es2020")

        if self.suite_name == "declgen-ets2ts-sdk":
            tsconfig_file = path.join(path.dirname(test_dets), path.basename(test_dets) + "_tsconfig.json")
            with os_fdopen(os_open(tsconfig_file, O_WRONLY|O_CREAT, 0o644), "w", encoding="utf-8") as handler:
                tsconfig = f"""\
                {{
                    "compilerOptions": {{
                    "baseUrl": "{self.decl_path}",
                    "paths": {{
                        "@ohos.base": [
                            "{self.decl_path}/api/@ohos.base.d.ets"
                        ]
                    }}
                }},
                "files" : [
                    "{test_dets}"
                ]
                }}"""
                handler.write(tsconfig)
            tsc_flags.append("--project")
            tsc_flags.append(tsconfig_file)
        else:
            tsc_flags.append(test_dets)

        params = Params(
            executor=self.tsc_executor,
            flags=tsc_flags,
            env=self.test_env.cmd_env,
            timeout=self.tsc_timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.TSC_FAIL,
            fail_kind_timeout=FailKind.TSC_TIMEOUT,
            fail_kind_other=FailKind.TSC_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="tsc",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_tsc(rc)
        )
        return passed, report, fail_kind

    def _validate_declgen_ets2ts(self, return_code: int, output_path: str) -> bool:
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    def _validate_tsc(self, return_code: int) -> bool:
        return return_code == 0

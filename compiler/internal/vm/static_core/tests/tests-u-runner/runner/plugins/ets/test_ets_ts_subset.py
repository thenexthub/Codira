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

from __future__ import annotations

import logging
from os import path, makedirs
from typing import Tuple, Optional, List
import shutil

from runner.logger import Log
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.plugins.ets.test_ets import TestETS

_LOGGER = logging.getLogger("runner.plugins.ets.test_ets_ts_subset")


class TestTSSubset(TestETS):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestETS.__init__(self, test_env, test_path, flags, test_id)

        self.test_ohos_ts = path.join(self.bytecode_path, f"{self.test_id}.ohos.ts")
        self.test_ohos_js = path.join(self.bytecode_path, f"{self.test_id}.ohos.js")
        self.tsconfig_json = path.join(self.test_env.work_dir.gen, "tsconfig.json")
        self.npx_folder = path.dirname(self.test_ohos_ts)
        makedirs(self.npx_folder, exist_ok=True)
        shutil.copyfile(self.path, self.test_ohos_ts, follow_symlinks=True)
        shutil.copymode(self.path, self.test_ohos_ts, follow_symlinks=True)

        self.tsc = "tsc"
        self.npx = "npx"
        self.node = "node"
        self.ohos_report: Optional[TestReport] = None
        self.node_report: Optional[TestReport] = None

    @property
    def _tsc_options(self) -> List[str]:
        return [
            "--target",
            "esnext",
            "--esModuleInterop",
            "--forceConsistentCasingInFileNames",
            "--alwaysStrict",
            "--strict",
            "--skipLibCheck",
            "--outFile",
            self.test_ohos_js,
            self.test_ohos_ts
        ]

    @property
    def _npx_options(self) -> List[str]:
        return [
            "--prefix",
            self.npx_folder,
            "ts-node",
            "-P",
            self.tsconfig_json,
            self.test_ohos_ts
        ]

    @property
    def _node_options(self) -> List[str]:
        return [
            self.test_ohos_js,
        ]

    def do_run(self) -> TestTSSubset:
        TestETS.do_run(self)
        if self.report:
            panda_report: TestReport = self.report

            self.ohos_workflow()
            self._compare_reports(
                l_report=panda_report.output,
                l_name="Panda",
                r_report=self.ohos_report.output if self.ohos_report else "",
                r_name="TS Node"
            )
        else:
            Log.default(_LOGGER, "Panda workflow have not generated any report. So nothing to compare")

        return self

    def ohos_workflow(self) -> TestTSSubset:
        steps = [
            self._run_ts_node,
        ]

        for step in steps:
            if self.passed:
                self.passed, self.ohos_report, self.fail_kind = step()

        return self

    def _compare_reports(self, l_report: str, l_name: str, r_report: str, r_name: str) -> None:
        if not self.passed:
            return
        self.log_cmd(f"Run compare outputs for {l_name} and {r_name}")
        self.passed = l_report.replace(" ", "") == r_report.replace(" ", "")
        if not self.passed:
            self.fail_kind = FailKind.COMPARE_FAIL
            report = ("Outputs NOT equal\n"
                      f"{l_name.capitalize()} output: '{l_report}'\n"
                      f"{r_name.capitalize()} output: '{r_report}'")
            self.report = TestReport(
                output=report,
                error="Outputs NOT equal",
                return_code=-1
            )
        else:
            self.report = TestReport(
                output="",
                error="",
                return_code=0
            )

        self.log_cmd(f"Output: '{self.report.output}'")
        self.log_cmd(f"Error: '{self.report.error}'")
        self.log_cmd(f"Return code: {self.report.return_code}")

    def _run_ts_node(self) -> Tuple[bool, TestReport, Optional[FailKind]]:
        params = Params(
            timeout=self.test_env.config.es2panda.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            executor=self.npx,
            flags=self._npx_options,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.TS_NODE_FAIL,
            fail_kind_timeout=FailKind.TS_NODE_TIMEOUT,
            fail_kind_other=FailKind.TS_NODE_OTHER,
        )

        return self.run_one_step(
            name="ts_node",
            params=params,
            result_validator=lambda _, _2, rc: self._ts_node_result_validator(rc)
        )

    def _run_node(self) -> Tuple[bool, TestReport, Optional[FailKind]]:
        params = Params(
            timeout=self.test_env.config.es2panda.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            executor=self.node,
            flags=self._node_options,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.NODE_FAIL,
            fail_kind_timeout=FailKind.NODE_TIMEOUT,
            fail_kind_other=FailKind.NODE_OTHER,
        )

        self.passed, self.node_report, self.fail_kind = self.run_one_step(
            name="node",
            params=params,
            result_validator=self._runtime_result_validator
        )

        return self.passed, self.node_report, self.fail_kind

#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from pathlib import Path

from typing_extensions import Self

from runner.enum_types.params import BinaryParams, TestEnv, TestReport
from runner.extensions.flows.test_flow_registry import ITestFlow
from runner.extensions.validators.base_validator import BaseValidator
from runner.logger import Log
from runner.options.options import IOptions
from runner.options.step import Step
from runner.suites.one_test_runner import OneTestRunner
from runner.suites.test_metadata import TestMetadata
from runner.suites.test_standard_flow import StandardFlowUtils
from runner.test_base import GTest

_LOGGER = Log.get_logger(__file__)


class GTestFlow(ITestFlow, GTest):
    gtest_class: str
    gtest_name: str
    build_dir: Path
    test_env: TestEnv
    passed: bool | None
    is_negative_runtime: bool
    fail_kind: str | None
    report: TestReport | None
    reproduce: str
    flow_utils: StandardFlowUtils

    def __init__(self, test_env: TestEnv, test_path: Path, *,
                 params: IOptions, test_id: str, precompiled_tests: bool):
        super().__init__(test_env, test_path, params=params, test_id=test_id, precompiled_tests=precompiled_tests)
        self._is_valid_test = True
        self.metadata = TestMetadata.create_empty_metadata(test_path, find_package=False)
        self._is_negative_compile = False
        self.is_negative_runtime = False
        self.gtest_abc = self.test_env.config.test_suite.get_parameter("gtest-abc")
        self.flow_utils = StandardFlowUtils(test_env)

    @property
    def is_valid_test(self) -> bool:
        return self._is_valid_test

    @property
    def is_negative_compile(self) -> bool:
        return self._is_negative_compile

    def do_run(self) -> Self:
        steps = list(self.test_env.config.workflow.steps)
        if not steps:
            self.passed = None
            self.fail_kind = 'None'

        for step in steps:
            passed, report, fail_kind = self._do_run_one_step(step)
            self.passed = passed
            self.report = report
            self.fail_kind = fail_kind

            if not self.passed:
                return self

        return self

    def _do_run_one_step(self, step: Step) -> tuple[bool, TestReport | None, str | None]:
        passed, report, fail_kind = self.__run_step(step)
        return passed, report, fail_kind

    def _make_params(self, step: Step, cmd_env: dict[str, str]) -> BinaryParams:
        if self.gtest_abc is not None:
            path_to_abc = Path(self.gtest_abc)
            test_vars = self._set_env_zip_for_test(cmd_env, path_to_abc)
        else:
            test_vars = cmd_env

        params = BinaryParams(
            executor=self.path,
            flags=[f"--gtest_filter={self.gtest_class}.{self.gtest_name}"],
            env=test_vars,
            timeout=step.timeout,
            use_qemu=not step.skip_qemu
        )

        return params

    def _set_env_zip_for_test(self, step_env: dict[str, str], path_to_abc: Path) -> dict[str, str]:
        test_env_zip = path_to_abc / f"{self.path.stem}_gtest_package.zip"
        if test_env_zip.exists():
            test_env_var: dict[str, str] = {"ANI_GTEST_ABC_PATH": test_env_zip.as_posix()}
            step_env.update(test_env_var)
        return step_env

    # NOTE: duplicate with TestStandardFlow._run_step — intentional.
    # They may diverge later (different flow-specific logic).
    # pylint: disable=duplicate-code
    def __run_step(self, step: Step) -> tuple[bool, TestReport, str | None]:
        cmd_env = self.flow_utils.get_step_env(step)
        params = self._make_params(step, cmd_env)

        test_runner = OneTestRunner(self.test_env)
        passed, report, fail_kind = test_runner.run_with_coverage(
            name=step.name,
            step_kind=step.step_kind,
            params=params,
            result_validator=BaseValidator.gtest_result_validator)
        self.reproduce += test_runner.reproduce
        return passed, report, fail_kind

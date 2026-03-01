#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import subprocess
import tempfile
from collections.abc import Callable
from copy import deepcopy
from pathlib import Path
from subprocess import CompletedProcess
from typing import cast

from runner.enum_types.fail_kind import FailureReturnCode
from runner.enum_types.params import BinaryParams, TestEnv, TestReport
from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.logger import Log
from runner.options.step import StepKind

_LOGGER = Log.get_logger(__file__)

GTestResultValidator = Callable[[str, CompletedProcess], ValidationResult]
ResultValidator = Callable[[str, str, int], ValidationResult]
CommonResultValidator = Callable[..., ValidationResult]  # type: ignore[explicit-any]
ReturnCodeInterpreter = Callable[[str, str, int], int]


class OneTestRunner:
    def __init__(self, test_env: TestEnv) -> None:
        self.test_env = test_env
        self.reproduce = ""
        self.coverage_config = self.test_env.config.general.coverage
        self.coverage_manager = test_env.coverage

    @staticmethod
    def __fail_kind_fail(name: str) -> str:
        return f"{name.upper()}_FAIL"

    @staticmethod
    def __fail_kind_neg_fail(name: str) -> str:
        return f"{name.upper()}_NEG_FAIL"

    @staticmethod
    def __fail_kind_segfault(name: str) -> str:
        return f"{name.upper()}_SEGFAULT_FAIL"

    @staticmethod
    def __fail_kind_abort_fail(name: str) -> str:
        return f"{name.upper()}_ABORT_FAIL"

    @staticmethod
    def __fail_kind_irtoc_assert_fail(name: str) -> str:
        return f"{name.upper()}_IRTOC_ASSERT_FAIL"

    @staticmethod
    def __fail_kind_other(name: str) -> str:
        return f"{name.upper()}_OTHER"

    @staticmethod
    def __fail_kind_subprocess(name: str) -> str:
        return f"{name.upper()}_SUBPROCESS"

    @staticmethod
    def __fail_kind_timeout(name: str) -> str:
        return f"{name.upper()}_TIMEOUT"

    @staticmethod
    def __failed_kind_special(name: str, kind: str) -> str:
        return f"{name.upper()}_{kind.upper()}"

    @staticmethod
    def __fail_kind_passed(name: str) -> str:
        return f"{name.upper()}_PASSED"

    def run_with_coverage(self, name: str, step_kind: StepKind, params: BinaryParams,
                          result_validator: CommonResultValidator,
                          return_code_interpreter: ReturnCodeInterpreter = lambda _, _2, rtc: rtc) \
            -> tuple[bool, TestReport, str | None]:

        coverage_per_binary = self.coverage_config.coverage_per_binary
        profraw_file, profdata_file, params = self.__get_prof_files(name, params)

        if self.coverage_config.use_lcov and coverage_per_binary:
            gcov_prefix, gcov_prefix_strip = self.coverage_manager.lcov_tool.get_gcov_prefix(params.component_name)
            params = deepcopy(params)
            params.env['GCOV_PREFIX'] = gcov_prefix
            params.env['GCOV_PREFIX_STRIP'] = gcov_prefix_strip

        passed, report, fail_kind = self.run_one_step(name, step_kind, params,
                                                      result_validator, return_code_interpreter)

        if self.coverage_config.use_llvm_cov and profraw_file and profdata_file:
            self.coverage_manager.llvm_cov_tool.merge_and_delete_profraw_files(profraw_file, profdata_file)

        return passed, report, fail_kind

    def run_one_step(self, name: str, step_kind: StepKind, params: BinaryParams,
                     result_validator: CommonResultValidator,
                     return_code_interpreter: ReturnCodeInterpreter = lambda _, _2, rtc: rtc) \
            -> tuple[bool, TestReport, str | None]:

        passed = False
        output = ""
        error = ""

        try:
            if step_kind == StepKind.GTEST_RUNNER:
                (passed, fail_kind, output,
                 error, return_code) = self.__run_gtest(name, params, cast(GTestResultValidator, result_validator))
            else:
                passed, fail_kind, output, error, return_code = self.__run(name, params,
                                                                           cast(ResultValidator, result_validator),
                                                                           return_code_interpreter)
        except subprocess.SubprocessError as ex:
            fail_kind = self.__fail_kind_subprocess(name)
            fail_kind_msg = f"{name}: Failed with {str(ex).strip()}"
            error = f"{error}\n{fail_kind_msg}" if error else fail_kind_msg
            return_code = -1
        self.__log_cmd(f"{name}: Actual error: {error.strip()}")
        self.__log_cmd(f"{name}: Actual return code: {return_code}\n")
        if fail_kind is None:
            fail_kind = self.__fail_kind_passed(name) if passed else self.__fail_kind_other(name)

        report = TestReport(
            output=output,
            error=error,
            return_code=return_code
        )

        return passed, report, fail_kind.upper() if fail_kind else fail_kind

    def __log_cmd(self, cmd: str) -> None:
        self.reproduce += f"\n{cmd}"

    def __run(self, name: str, params: BinaryParams, result_validator: ResultValidator,
              return_code_interpreter: ReturnCodeInterpreter) -> tuple[bool, str | None, str, str, int]:
        cmd = [str(params.executor)]
        if params.use_qemu:
            cmd = [*self.test_env.cmd_prefix, *cmd]
        cmd.extend(params.flags)
        passed = False
        output = ""

        self.__log_cmd(f"{name}: {' '.join(cmd)}")
        _LOGGER.all(f"Run {name}: {' '.join(cmd)}")

        with (subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=params.env,
                encoding='utf-8',
                errors='ignore',
        ) as process):
            fail_kind: str | None = None
            try:
                output, error = process.communicate(timeout=params.timeout)
                return_code = return_code_interpreter(output, error, process.returncode)
                validation_result = result_validator(output, error, return_code)
                passed = validation_result.passed
                self.__log_cmd(f"{name}: Actual output: {output.strip()}")
                if not passed:
                    if validation_result.kind in [ValidatorFailKind.COMPARE_OUTPUT,
                                                    ValidatorFailKind.STDERR_NOT_EMPTY]:
                        kind = validation_result.kind.value
                        fail_kind = self.__failed_kind_special(name, kind)

                    else:
                        fail_kind = self._detect_fail_kind(name, return_code)

            except subprocess.TimeoutExpired:
                self.__log_cmd(f"{name}: Failed by timeout after {params.timeout} sec")
                fail_kind = self.__fail_kind_timeout(name)
                error = fail_kind
                return_code = process.returncode
                process.kill()
        return passed, fail_kind, output, error, return_code

    def __run_gtest(self, name: str,
                    params: BinaryParams,
                    result_validator: GTestResultValidator) -> tuple[bool, str | None, str, str, int]:
        cmd = [str(params.executor)]
        if params.use_qemu:
            cmd = [*self.test_env.cmd_prefix, *cmd]
        cmd.extend(params.flags)
        passed = False
        fail_kind: str | None = None

        self.__log_cmd(f"{name}: {' '.join(cmd)}")
        _LOGGER.all(f"Run {name}: {' '.join(cmd)}")

        with tempfile.NamedTemporaryFile(mode='w', suffix='.json') as f:
            json_file = f.name

            try:
                result = subprocess.run(
                    [*cmd, f"--gtest_output=json:{json_file}"],
                    capture_output=True,
                    text=True,
                    timeout=params.timeout,
                    env=params.env,
                    check=False
                )
                output = result.stdout
                error = result.stderr
                return_code = result.returncode
                validator_result = result_validator(json_file, result)
                passed = validator_result.passed
                fail_kind = validator_result.kind.value
                self.__log_cmd(f"{name}: Actual output: {output.strip()}")
            except subprocess.TimeoutExpired as pt:
                self.__log_cmd(f"{name}: Failed by timeout after {params.timeout} sec")
                fail_kind = self.__fail_kind_timeout(name)
                error = pt.stderr.decode() if pt.stderr else ""
                output = pt.stdout.decode() if pt.stdout else ""
                return_code = -1

        return passed, fail_kind, output, error, return_code

    def __get_prof_files(
            self,
            name: str,
            params: BinaryParams
    ) -> tuple[Path | None, Path | None, BinaryParams]:
        profraw_file, profdata_file = None, None
        llvm_cov_tool = self.coverage_manager.llvm_cov_tool

        if not self.coverage_config.use_llvm_cov:
            return profraw_file, profdata_file, params

        if self.coverage_config.coverage_per_binary:
            profraw_file, profdata_file = llvm_cov_tool.get_uniq_profraw_profdata_file_paths(name)
        else:
            profraw_file, profdata_file = llvm_cov_tool.get_uniq_profraw_profdata_file_paths()

        params = deepcopy(params)
        params.env['LLVM_PROFILE_FILE'] = str(profraw_file)

        return profraw_file, profdata_file, params

    def _detect_fail_kind(self, name: str, return_code: int) -> str:
        if return_code in FailureReturnCode.SEGFAULT_RETURN_CODE.value:
            return self.__fail_kind_segfault(name)
        if return_code in FailureReturnCode.ABORT_RETURN_CODE.value:
            return self.__fail_kind_abort_fail(name)
        if return_code in FailureReturnCode.IRTOC_ASSERT_RETURN_CODE.value:
            return self.__fail_kind_irtoc_assert_fail(name)
        if return_code == 0:
            return self.__fail_kind_neg_fail(name)
        return self.__fail_kind_fail(name)

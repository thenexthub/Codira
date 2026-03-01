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
import json
import re
from json import JSONDecodeError
from subprocess import CompletedProcess
from typing import TYPE_CHECKING

from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.step import StepKind
from runner.utils import normalize_str

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)


class BaseValidator(IValidator):
    def __init__(self, add_base_validators: bool = True) -> None:
        super().__init__()
        if add_base_validators:
            for value in StepKind.values():
                self.add(value, BaseValidator.check_return_code)
                self.add(value, BaseValidator.check_stderr)

    @staticmethod
    def check_return_code(test: "TestStandardFlow", step_value: str, output: str,
                            error_output: str, return_code: int) -> ValidationResult:
        if test.ignored and test.last_failure:
            test.last_failure_check_passed = BaseValidator._check_last_failure_for_ignored_test(test.last_failure,
                                                                                                output, error_output)

        step_passed = BaseValidator._step_passed(test, return_code, step_value)
        if step_passed:
            return ValidationResult(True, ValidatorFailKind.NONE, "")
        return ValidationResult(False, ValidatorFailKind.STEP_FAILED, "")

    @staticmethod
    def check_stderr(test: "TestStandardFlow", step_value: str, _1: str,
                            error_output: str, _2: int) -> ValidationResult:

        if error_output.strip():
            _LOGGER.all(f"Step validator {step_value}: get error output '{error_output}'")
        # if output.find('[Fail]Device not founded or connected') > -1:
        #     return ValidationResult(False, ValidatorFailKind.DEVICE_NOT_FOUND, "Device not founded or connected")

        if not error_output:
            return ValidationResult(True, ValidatorFailKind.NONE, "")

        if test.has_expected_err:
            comparison_res = test.compare_with_stderr(error_output)
            if comparison_res:
                return ValidationResult(comparison_res, ValidatorFailKind.NONE, "")
            return ValidationResult(comparison_res, ValidatorFailKind.COMPARE_OUTPUT,
                                    "Comparison with .expected or .expected.err file has failed")

        return ValidationResult(False, ValidatorFailKind.STDERR_NOT_EMPTY, "stderr is not empty")

    @staticmethod
    def check_output(test: "TestStandardFlow", step_value: str, output: str,
                            _1: str, _2: int) -> ValidationResult:
        comparison_res = test.compare_with_stdout(output)
        if step_value == StepKind.COMPILER.value:
            fail_kind = ValidatorFailKind.NONE if comparison_res else ValidatorFailKind.COMPARE_OUTPUT
            return ValidationResult(comparison_res, fail_kind, "")

        if comparison_res:
            return ValidationResult(comparison_res, ValidatorFailKind.NONE, f"{step_value} validation passed",
                                    stop_runtime_validation=True)

        test.runtime_steps.pop(0)
        if test.runtime_steps and not output:
            # if there are more runtime steps - comparison may happen there
            return ValidationResult(True, ValidatorFailKind.NONE, "")

        # if there are no more runtime steps or
        # current runtime step had expected file to compare output with and the comparison failed
        # - return the comparison_res
        return ValidationResult(comparison_res, ValidatorFailKind.COMPARE_OUTPUT,
                                 "Comparison with .expected or .expected.err file has failed")

    @staticmethod
    def gtest_result_validator(json_file: str, test_result: CompletedProcess) -> ValidationResult:
        passed = False

        # should be addedcheck: if there is something in stderr check if it was expected
        # if not - test should fail

        with open(json_file, encoding='utf-8') as f:
            try:
                test_data = json.load(f)
            except JSONDecodeError:
                print(test_result)

            for test in test_data['testsuites']:
                for test_case in test['testsuite']:
                    passed = bool(test_case['status'] == 'RUN' and test_case['result'] == 'COMPLETED')

        if passed and test_result.returncode == 0:
            return ValidationResult(True, ValidatorFailKind.NONE, "")
        return ValidationResult(False, ValidatorFailKind.STEP_FAILED, "")

    @staticmethod
    def _step_passed(test: "TestStandardFlow", return_code: int, step: str) -> bool:
        if step == StepKind.COMPILER.value:
            if test.is_negative_compile:
                return return_code == 1
            return return_code == 0

        if step == StepKind.RUNTIME.value:
            if test.is_negative_runtime:
                return return_code in [1, 255]
            return return_code == 0

        return return_code == 0

    @staticmethod
    def _check_last_failure_for_ignored_test(failure: str, output: str, error_output: str) -> bool:
        output = normalize_str(output)
        error_output = normalize_str(error_output)
        failure = normalize_str(failure)

        if output:
            return bool(re.search(failure, output))
        if error_output:
            return bool(re.search(failure, error_output))

        return False

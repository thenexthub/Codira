#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

import json
from json import JSONDecodeError, JSONDecoder

from runner import common_exceptions, utils
from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.validators.astchecker.util_astchecker import UtilASTChecker
from runner.extensions.validators.base_validator import BaseValidator
from runner.logger import Log
from runner.options.step import StepKind
from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)


class AstCheckerValidator(BaseValidator):
    __util = UtilASTChecker()

    def __init__(self) -> None:
        super().__init__()
        self.add(StepKind.COMPILER.value, AstCheckerValidator.es2panda_result_validator)

    @staticmethod
    def handle_error_dump(input_string: str) -> tuple[str, dict]:
        actual_output = ""
        errors_list = []
        not_handled_errors = []
        lines = input_string.splitlines()
        for line in lines:
            if line.startswith("Warning") or line.startswith("SyntaxError") or line.startswith("TypeError"):
                errors_list.append(line)  # Add the line to the errors list
            if line.startswith("Error:"):
                not_handled_errors.append(line)

        errors = "\n".join(errors_list)
        errors_list.extend(not_handled_errors)

        filtered_lines = [line for line in lines if line not in errors_list]
        dump = "\n".join(filtered_lines)

        if any(char != '\b' for char in dump):
            decoder = JSONDecoder()
            pos = 0
            try:
                obj, _ = decoder.raw_decode(dump, pos)
                obj = json.loads(json.dumps(obj))
            except JSONDecodeError:
                obj = {}
            return errors, obj

        return errors, json.loads(actual_output) if actual_output else {}

    @staticmethod
    def es2panda_result_validator(test: object, step_name: str, actual_output: str, _2: str,
                                  return_code: int) -> ValidationResult:
        error = ""
        dump: dict[str, str | list | dict] = {}
        if not isinstance(test, TestStandardFlow):
            raise common_exceptions.UnknownException(
                "AstCheckerValidator works only with tests of TestStandardFlow inherits. "
                f"Current type of test {type(test)}"
            )

        try:
            error, dump = AstCheckerValidator.handle_error_dump(actual_output)
        except ValueError as err:
            raise UtilASTChecker.AstCheckerException(
                f'Output from file: {test.path}.\nThrows JSON error: {err}.') from err

        test_cases = AstCheckerValidator.__util.parse_tests(utils.read_file(test.path))
        passed = AstCheckerValidator.__util.run_tests(test.path, test_cases, dump, error=error)

        if test_cases.has_error_tests and not test_cases.skip_errors:
            passed = passed and return_code != 0
        else:
            passed = passed and return_code == 0

        fail_kind_other = f"{step_name.upper()}_OTHER"
        passed = ((test_cases.has_error_tests or test_cases.has_warning_tests)
                  and test.fail_kind == fail_kind_other) ^ passed

        return ValidationResult(passed, ValidatorFailKind.NONE, fail_kind_other)

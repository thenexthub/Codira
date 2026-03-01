#!/usr/bin/env python3
# -- coding: utf-8 --
#
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
from collections.abc import Callable
from typing import TYPE_CHECKING

from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.validators.base_validator import BaseValidator
from runner.logger import Log
from runner.options.step import StepKind

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)

ValidatorFunction = Callable[["TestStandardFlow", str, str, str, int], ValidationResult]


class ParserValidator(BaseValidator):

    def __init__(self) -> None:

        super().__init__(add_base_validators=False)

        for value in StepKind.values():
            self.add(value, ParserValidator.es2panda_result_validator)

    @staticmethod
    def es2panda_result_validator(test: "TestStandardFlow", _: str, actual_output: str, _2: str,
                                  return_code: int) -> ValidationResult:
        fail_kind = ValidatorFailKind.NONE
        try:
            expected_name = f"{test.path.stem}-expected"
            expected_path = test.path.with_stem(expected_name).with_suffix(".txt")
            with open(expected_path, encoding="utf-8") as file_pointer:
                expected = file_pointer.read()
            passed = expected == actual_output and return_code in [0, 1]
            if not passed:
                fail_kind = ValidatorFailKind.COMPARE_OUTPUT
        except OSError:
            passed = False

        return ValidationResult(passed, fail_kind, "")

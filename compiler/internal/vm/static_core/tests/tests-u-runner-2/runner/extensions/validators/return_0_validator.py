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
from typing import TYPE_CHECKING

from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.step import StepKind

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)


class Return0Validator(IValidator):
    def __init__(self) -> None:
        super().__init__()
        for value in StepKind.values():
            self.add(value, Return0Validator.check_return_code_0)

    @staticmethod
    def passed() -> ValidationResult:
        return ValidationResult(passed=True)

    @staticmethod
    def failed(return_code: int) -> ValidationResult:
        return ValidationResult(
            passed=False,
            kind=ValidatorFailKind.STEP_FAILED,
            description=f"Return code expected 0, but actual value is {return_code}"
        )

    @classmethod
    def check_return_code_0(cls, _: "TestStandardFlow", _2: str, _3: str,
                            _4: str, return_code: int) -> ValidationResult:
        return cls.passed() if return_code == 0 else cls.failed(return_code)

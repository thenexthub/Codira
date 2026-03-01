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

from abc import ABC
from collections.abc import Callable
from typing import TYPE_CHECKING

from runner.enum_types.validation_result import ValidationResult

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

# Signature:
# test: TestStandardFlow - what test is running
# step_name: str - what step is running
# output: str - what stdout contains
# error: str - what stderr contains
# return_code: int - return code of the binary run on the step for the test
ValidatorFunction = Callable[['TestStandardFlow', str, str, str, int], ValidationResult]


class IValidator(ABC):  # noqa: B024

    def __init__(self) -> None:
        self._validators: dict[str, list[ValidatorFunction]] = {}

    @property
    def validators(self) -> dict[str, list[ValidatorFunction]]:
        return self._validators

    def add(self, step_name: str, function: ValidatorFunction) -> None:
        if step_name not in self._validators:
            self._validators[step_name] = []
        self._validators[step_name].append(function)

    def delete(self, step_name: str, function: ValidatorFunction) -> None:
        step_validators = self._validators[step_name]
        if function in step_validators:
            step_validators.remove(function)
        self._validators[step_name] = step_validators

    def has_step(self, step_name: str) -> bool:
        return step_name in self._validators

    def get_validator(self, step_name: str) -> list[ValidatorFunction] | None:
        return self._validators.get(step_name)

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

from dataclasses import dataclass
from pathlib import Path
from typing import Any, cast

from runner import utils
from runner.common_exceptions import IncorrectEnumValue, MalformedStepConfigurationException
from runner.enum_types.base_enum import BaseEnum
from runner.logger import Log
from runner.options.options import IOptions
from runner.utils import to_bool

_LOGGER = Log.get_logger(__file__)

PropType = str | int | bool | list[str] | dict[str, Any] | BaseEnum  # type: ignore[explicit-any]


class StepKind(BaseEnum):
    COMPILER = "compiler"
    VERIFIER = "verifier"
    AOT = "aot"
    RUNTIME = "runtime"
    GTEST_RUNNER = "gtest-runner"
    OTHER = "other"


@dataclass
class Step(IOptions):
    name: str
    timeout: int
    args: list[str]
    env: dict[str, str]
    step_kind: StepKind
    step_filter: str = "*"
    enabled: bool = True
    executable_path: Path | None = None
    can_be_instrumented: bool = False
    default_timeout: int = 30
    skip_qemu: bool = False

    __MIN_ARG_LENGTH = 15
    __MAX_ARG_LENGTH = 150
    __INDENT_STEP = "\t"
    __INDENT_ARG = __INDENT_STEP * 2
    __INDENT_SUB_ARG = __INDENT_STEP * 3

    def __init__(self, name: str, step_body: dict[str, Any]):  # type: ignore[explicit-any]
        super().__init__()
        self.name = name
        self.executable_path = self.__get_path_property(step_body, 'executable-path')
        self.timeout = self.__get_int_property(step_body, 'timeout', self.default_timeout)
        self.can_be_instrumented = self.__get_bool_property(step_body, 'can-be-instrumented', False)
        self.enabled = self.__get_bool_property(step_body, 'enabled', True)
        self.step_kind = self.__get_kind_property(step_body, 'step-type', StepKind.OTHER)
        self.args = self.__get_list_property(step_body, 'args', [])
        self.skip_qemu = self.__get_bool_property(step_body, 'skip-qemu', False)
        self.env = self.__get_dict_property(step_body, 'env', {})
        self.step_filter = self.__get_str_property(step_body, 'step-filter', '*')

    def __str__(self) -> str:
        indent = 3
        result = [f"{utils.indent(indent)}{self.name}:"]
        indent += 1
        result += [f"{utils.indent(indent)}executable-path: {self.executable_path}"]
        result += [f"{utils.indent(indent)}timeout: {self.timeout}"]
        result += [f"{utils.indent(indent)}can-be-instrumented: {self.can_be_instrumented}"]
        result += [f"{utils.indent(indent)}enabled: {self.enabled}"]
        result += [f"{utils.indent(indent)}skip-qemu: {self.skip_qemu}"]
        result += [f"{utils.indent(indent)}step-type: {self.step_kind}"]
        result += [f"{utils.indent(indent)}step-filter: {self.step_filter}"]

        if self.args:
            result += self.__str_process_args(indent)

        if self.env:
            result += self.__str_process_env(indent)
        return "\n".join(result)

    @staticmethod
    def __get_int_property(step_body: dict[str, Any],   # type: ignore[explicit-any]
                           name: str, default: int | None = None) -> int:
        value = Step.__get_property(step_body, name, default)
        try:
            return int(value)
        except ValueError:
            return False

    @staticmethod
    def __get_bool_property(step_body: dict[str, Any],  # type: ignore[explicit-any]
                            name: str, default: bool | None = None) -> bool:
        value = Step.__get_property(step_body, name, default)
        try:
            return to_bool(value)
        except ValueError as exc:
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'") from exc

    @staticmethod
    def __get_list_property(step_body: dict[str, Any], name: str,   # type: ignore[explicit-any]
                            default: list[str] | None = None) -> list[str]:
        value = Step.__get_property(step_body, name, default)
        if not isinstance(value, list):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      "Expected list.")
        return cast(list[str], value)

    @staticmethod
    def __get_dict_property(step_body: dict[str, Any], name: str,   # type: ignore[explicit-any]
                            default: dict[str, Any] | None = None) -> dict[str, str]:
        value = Step.__get_property(step_body, name, default)
        if not isinstance(value, dict):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      "Expected dict.")
        return cast(dict[str, str], value)

    @staticmethod
    def __get_str_property(step_body: dict[str, Any], name: str,     # type: ignore[explicit-any]
                           default: str | None = None) -> str:
        value = Step.__get_property(step_body, name, default)
        if isinstance(value, (dict | list)):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      "Expected str.")
        return str(value)

    @staticmethod
    def __get_path_property(step_body: dict[str, Any], name: str) -> Path | None:  # type: ignore[explicit-any]
        value = Step.__get_str_property(step_body, name, "")
        if value == "":
            return None
        value_path = Path(value)
        return value_path

    @staticmethod
    def __get_kind_property(step_body: dict[str, Any], name: str,   # type: ignore[explicit-any]
                            default: StepKind | None = None) -> StepKind:
        value = Step.__get_property(step_body, name, default)
        try:
            if isinstance(value, StepKind):
                return value
            return StepKind.is_value(value, option_name="'step-type' from workflow config")
        except IncorrectEnumValue as exc:
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      f"Expected one of {StepKind.values()}.") from exc

    @staticmethod
    def __get_property(step_body: dict[str, Any],  # type: ignore[explicit-any]
                       name: str,
                       default: PropType | None = None) -> \
            Any:
        if name in step_body:
            value = step_body[name]
        elif default is not None:
            value = default
        else:
            raise MalformedStepConfigurationException(f"missed required field '{name}'")
        return value

    def executable_path_exists(self) -> bool:
        if self.executable_path is None:
            return False
        return bool(self.executable_path.exists() and self.executable_path.is_file())

    def pretty_str(self) -> str:
        if not str(self.executable_path):
            return ''
        result = f"Step '{self.name}':"
        args: list[str] = [self.__pretty_arg_str(arg) for arg in self.args]
        args_str = '\n'.join(args)
        result += f"\n{self.__INDENT_STEP}{self.executable_path}\n{args_str}"
        return result

    def __pretty_arg_str(self, arg: str) -> str:
        args: list[str] = []
        arg = str(arg)
        if len(arg) < self.__MAX_ARG_LENGTH:
            args.append(f"{self.__INDENT_ARG}{arg}")
        else:
            sub_args = arg.split(" ")
            args.extend(self.__pretty_sub_arg(sub_args))
        return '\n'.join(args)

    def __pretty_sub_arg(self, sub_args: list[str]) -> list[str]:
        acc = ''
        result: list[str] = []
        for sub_arg in sub_args:
            if len(sub_arg) > self.__MIN_ARG_LENGTH:
                if len(acc) > 0:
                    result.append(f"{self.__INDENT_SUB_ARG}{acc}")
                    acc = ""
                result.append(f"{self.__INDENT_SUB_ARG}{sub_arg}")
            else:
                acc += sub_arg + " "
        if len(acc) > 0:
            result.append(f"{self.__INDENT_SUB_ARG}{acc}")
        return result

    def __str_process_env(self, indent: int) -> list[str]:
        result = [f"{utils.indent(indent)}env:"]
        for key in self.env:
            env_value: str | list[str] = self.env[key]
            if isinstance(env_value, list):
                result += [f"{utils.indent(indent + 1)}{key}:"]
                for item in env_value:
                    result += [f"{utils.indent(indent + 2)}- {item}"]
            else:
                result += [f"{utils.indent(indent + 1)}{key}: {env_value}"]
        return result

    def __str_process_args(self, indent: int) -> list[str]:
        result = [f"{utils.indent(indent)}args:"]
        for arg in self.args:
            if arg.strip():
                result += [f"{utils.indent(indent + 1)}{arg}"]
        return result

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
import functools
import random
import unittest
from collections.abc import Callable
from pathlib import Path
from typing import Any, TypeVar
from unittest.mock import MagicMock

from runner import utils
from runner.common_exceptions import InvalidConfiguration
from runner.suites.test_lists import TestLists

MethodType = Any    # type: ignore[explicit-any]
CLASSTYPE = TypeVar("CLASSTYPE", bound='object')


def compare_dicts(test_case: unittest.TestCase, actual_dict: dict[str, Any],  # type: ignore[explicit-any]
                  expected_dict: dict[str, Any]) -> None:
    for key in actual_dict:
        test_case.assertTrue(key in expected_dict, f"Key {key} is absent in {expected_dict.keys()}")
        actual_value = actual_dict[key]
        expected_value = expected_dict[key]
        type_actual_value = type(actual_value)
        type_expected_value = type(expected_value)
        if type_actual_value != type_expected_value and isinstance(actual_value, str):
            actual_value = try_to_cast_one_type(actual_value, expected_value)
            type_actual_value = type(actual_value)
        test_case.assertEqual(type_expected_value, type_actual_value,
                              f"Key '{key}': type of expected value = {type_expected_value}, "
                              f"type of actual value = {type_actual_value}. Expected to be equal")

        if actual_value and expected_value and isinstance(actual_value, dict):
            compare_dicts(test_case, actual_value, expected_value)
        elif isinstance(actual_value, list):
            test_case.assertListEqual(sorted(actual_value), sorted(expected_value))
        else:
            test_case.assertEqual(
                expected_value,
                actual_value,
                f"Key '{key}': expected value = {expected_value}, "
                f"actual value = {actual_value}. Expected to be equal")


def try_to_cast_one_type(value1: str, value2: Any) -> int | bool | str:  # type: ignore[explicit-any]
    if isinstance(value2, int):
        return int(value1)
    if isinstance(value2, bool):
        return utils.to_bool(value1)
    return value1


def get_method(cls: type[CLASSTYPE], name: str) -> MethodType:
    for key, value in cls.__dict__.items():
        if key in [name, f"_{cls.__name__}{name}"] and callable(value):
            return value
    raise InvalidConfiguration(f"Cannot find method '{name}' at class CliOptions")


def assert_not_raise(test_case: unittest.TestCase, cls: type[Exception], function: Callable, params: list) -> None:
    exception_occurs = False
    exception_message = ""
    try:
        function(*params)
    except cls as ex:
        exception_occurs = True
        exception_message = f"Unexpected exception has occured: {ex}"
    finally:
        test_case.assertFalse(exception_occurs, exception_message)


def random_suffix() -> str:
    return str(round(random.random() * 1000_000))


def create_runner_test_id(test_file: str) -> str:
    current_file_parent = Path(__file__).parent
    return Path(test_file).relative_to(current_file_parent).as_posix()


def test_cmake_build(_: TestLists) -> list[str]:
    return [
        'PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER=false',
        'PANDA_ENABLE_ADDRESS_SANITIZER=true',
        'PANDA_ENABLE_THREAD_SANITIZER=false',
        'CMAKE_BUILD_TYPE=fastverify'
    ]


def test_gn_build(_: TestLists) -> list[str]:
    return [
        'is_asan=true',
        'is_fastverify=true'
    ]


def test_environ(test_root_name: str | None = None, test_root_value: str | None = None) -> dict[str, str]:
    env = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': (Path.cwd() / f"work-{random_suffix()}").as_posix(),
    }
    if test_root_name and test_root_value:
        env[test_root_name] = test_root_value
    return env


def data_folder(main_path: str, test_data_folder: str = "data") -> Path:
    return Path(main_path).parent / test_data_folder


def parametrized_test_cases(test_cases: list[tuple[list[str]]]) -> Callable:
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper(self: 'unittest.TestCase') -> None:
            for i, test_case in enumerate(test_cases):
                with self.subTest(case_index=i, args=test_case):
                    func(self, *test_case)
        return wrapper
    return decorator


def get_list_path(main_path: str, list_name: str, test_data_folder: str = "data") -> Path:
    return Path(main_path).parent / test_data_folder / list_name


def set_process_mock(mock_popen: MagicMock, return_code: int, output: str = "", error_out: str = "") -> None:
    proc = MagicMock()
    proc.__enter__.return_value = proc
    proc.communicate.return_value = (output, error_out)
    proc.returncode = return_code
    mock_popen.return_value = proc

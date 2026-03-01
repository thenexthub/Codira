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

import argparse
from functools import cached_property
from pathlib import Path
from typing import Any, cast

from runner.common_exceptions import InvalidConfiguration
from runner.logger import Log
from runner.options.macros import Macros, ParameterNotFound
from runner.options.options import IOptions
from runner.options.options_collections import CollectionsOptions
from runner.options.options_ets import ETSOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_groups import GroupsOptions
from runner.options.options_test_lists import TestListsOptions
from runner.utils import check_int, convert_underscore, extract_parameter_name

_LOGGER = Log.get_logger(__file__)


class TestSuiteOptions(IOptions):
    __DATA = "data"
    __TEST_SUITE = "test-suite"

    __PROPERTIES = "properties"
    __COLLECTIONS = "collections"
    __LIST_ROOT = "list-root"
    __TEST_ROOT = "test-root"

    __PARAMETERS = "parameters"
    __EXTENSION = "extension"
    __DEFAULT_EXTENSION = "ets"
    __FILTER = "filter"
    __DEFAULT_FILTER = "*"
    __LOAD_RUNTIMES = "load-runtimes"
    __DEFAULT_LOAD_RUNTIMES = "ets"
    __REPEATS = "repeats"
    __DEFAULT_REPEATS = 1
    __REPEATS_BY_TIME = "repeats-by-time"
    __DEFAULT_REPEATS_BY_TIME = 0
    __WITH_JS = "with-js"
    __DEFAULT_WITH_JS = False
    __SKIP_COMPILE_ONLY_NEG = "skip-compile-only-neg"
    __DEFAULT_SKIP_COMPILE_ONLY = False
    __WORK_DIR = "work-dir"
    __USE_METADATA = "use-metadata"
    __DEFAULT_USE_METADATA = True

    def __init__(self, args: dict[str, Any], parent: IOptions):  # type: ignore[explicit-any]
        super().__init__(None)
        self.__name: str = cast(str, args[self.__TEST_SUITE])
        if not isinstance(parent, GeneralOptions):
            raise InvalidConfiguration(
                "Incorrect configuration: test suite parent is not GeneralOptions")
        self._parent: GeneralOptions = parent
        self.__parameters: dict[str, Any] = self.__process_parameters(args)  # type: ignore[explicit-any]
        self.__data = args[f"{self.__name}.data"]
        self.__default_list_root: Path = self._parent.static_core_root / 'tests' / 'tests-u-runner' / 'test-lists'
        self.__list_root: str | None = self.__data[self.__LIST_ROOT] \
            if self.__data[self.__LIST_ROOT] else str(self.__default_list_root)
        self.__test_root: str | None = self.__data[self.__TEST_ROOT] \
            if self.__data[self.__TEST_ROOT] else None
        self.__collections: list[CollectionsOptions] = []
        self.test_lists = TestListsOptions(self.__parameters)
        self.ets = ETSOptions(self.__parameters)
        self.groups = GroupsOptions(self.__parameters)
        self.__expand_macros_in_parameters()
        self.__fill_collections()
        self.__expand_macros_collections(self._parent)

    def __str__(self) -> str:
        return self._to_str(indent=1)

    @property
    def list_root(self) -> Path:
        if self.__list_root is None:
            raise InvalidConfiguration("List-root is not specified")
        return Path(self.__list_root)

    @property
    def test_root(self) -> Path:
        if self.__test_root is None:
            raise InvalidConfiguration("Test-root is not specified")
        return Path(self.__test_root)

    @property
    def work_dir(self) -> str:
        if self.__WORK_DIR not in self.__parameters:
            raise InvalidConfiguration("work-dir is not specified")
        return str(self.__parameters[self.__WORK_DIR])

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        dest = f"{dest}." if dest else ""
        config = parser.add_argument_group(title="Test suite default options")
        config.add_argument(
            f'--{TestSuiteOptions.__FILTER}', '-f', action='store',
            default=TestSuiteOptions.__DEFAULT_FILTER,
            dest=f"{dest}{TestSuiteOptions.__FILTER}",
            help=f'test filter wildcard. By default \'{TestSuiteOptions.__DEFAULT_FILTER}\'')
        repeats_group = parser.add_mutually_exclusive_group(required=False)
        repeats_group.add_argument(
            f'--{TestSuiteOptions.__REPEATS}', action='store',
            type=lambda arg: check_int(arg, f"--{TestSuiteOptions.__REPEATS}", is_zero_allowed=False),
            default=TestSuiteOptions.__DEFAULT_REPEATS,
            dest=f"{dest}{TestSuiteOptions.__REPEATS}",
            help=f'how many times to repeat the suite entirely. By default {TestSuiteOptions.__DEFAULT_REPEATS}')
        repeats_group.add_argument(
            f'--{TestSuiteOptions.__REPEATS_BY_TIME}', action='store',
            type=lambda arg: check_int(arg, f"--{TestSuiteOptions.__REPEATS_BY_TIME}", is_zero_allowed=True),
            default=TestSuiteOptions.__DEFAULT_REPEATS_BY_TIME,
            dest=f"{dest}{TestSuiteOptions.__REPEATS_BY_TIME}",
            help=f'number of seconds during which the suite is repeated. '
                 f'Number of repeats is always integer. By default {TestSuiteOptions.__DEFAULT_REPEATS_BY_TIME}')
        config.add_argument(
            f'--{TestSuiteOptions.__WITH_JS}', action='store_true',
            default=TestSuiteOptions.__DEFAULT_WITH_JS,
            dest=f"{dest}{TestSuiteOptions.__WITH_JS}",
            help='enable JS-related tests')
        config.add_argument(
            f'--{TestSuiteOptions.__SKIP_COMPILE_ONLY_NEG}', action='store_true',
            default=TestSuiteOptions.__DEFAULT_SKIP_COMPILE_ONLY,
            dest=f"{dest}{TestSuiteOptions.__SKIP_COMPILE_ONLY_NEG}",
            help='if set the tests marked as `compile-only` are excluded from launch. '
                 'By default, all tests are launched.')

        TestListsOptions.add_cli_args(parser, dest)
        ETSOptions.add_cli_args(parser, dest)
        GroupsOptions.add_cli_args(parser, dest)

    @staticmethod
    def __fill_collection(content: dict) -> dict:
        for _, prop_value in content.items():
            if isinstance(prop_value, dict):
                TestSuiteOptions.__fill_collection_props(prop_value)
        return content

    @staticmethod
    def __fill_collection_props(prop_value: dict) -> None:
        for sub_key, sub_value in prop_value.items():
            if isinstance(sub_value, list):
                prop_value[sub_key] = " ".join(sub_value)

    @cached_property
    def suite_name(self) -> str:
        return self.__name

    @cached_property
    def collections(self) -> list[CollectionsOptions]:
        return self.__collections

    @cached_property
    def repeats(self) -> int:
        return int(self.__parameters.get(self.__REPEATS, self.__DEFAULT_REPEATS))

    @cached_property
    def repeats_by_time(self) -> int:
        return cast(int, self.__parameters.get(self.__REPEATS_BY_TIME, self.__DEFAULT_REPEATS_BY_TIME))

    @cached_property
    def filter(self) -> str:
        return str(self.__parameters.get(self.__FILTER, self.__DEFAULT_FILTER))

    @cached_property
    def use_metadata(self) -> bool:
        return bool(self.__parameters.get(self.__USE_METADATA, self.__DEFAULT_USE_METADATA))

    @cached_property
    def parameters(self) -> dict[str, Any]:  # type: ignore[explicit-any]
        return self.__parameters

    @cached_property
    def skip_compile_only_neg(self) -> bool:
        return cast(bool, self.get_parameter(self.__SKIP_COMPILE_ONLY_NEG, self.__DEFAULT_SKIP_COMPILE_ONLY))

    def extension(self, collection: CollectionsOptions | None = None) -> str:
        return str(self.get_parameter(self.__EXTENSION, self.__DEFAULT_EXTENSION, collection))

    def with_js(self, collection: CollectionsOptions | None = None) -> bool:
        return cast(bool, self.get_parameter(self.__WITH_JS, self.__DEFAULT_WITH_JS, collection))

    def load_runtimes(self, collection: CollectionsOptions | None = None) -> str:
        return str(self.get_parameter(self.__LOAD_RUNTIMES, self.__DEFAULT_LOAD_RUNTIMES, collection))

    def get_parameter(self, key: str, default: Any | None = None,   # type: ignore[explicit-any]
                      collection: CollectionsOptions | None = None) -> Any | None:
        if collection is not None:
            value = collection.get_parameter(key, None)
            if value is not None:
                return value
        return self.__parameters.get(key, default)

    def is_defined_in_collections(self, key: str) -> bool:
        key = extract_parameter_name(key)
        for collection in self.__collections:
            if key in collection.parameters:
                return True
        return False

    def get_command_line(self) -> str:
        options = [
            f'--test-root={self.test_root}' if self.test_root is not None else '',
            f'--list-root={self.list_root}' if self.list_root is not None else '',
        ]
        return ' '.join(options)

    def __fill_collections(self) -> None:
        if self.__COLLECTIONS not in self.__data:
            self.__collections.append(CollectionsOptions("", self.__data, self))
            return
        for coll_name, content in self.__data[self.__COLLECTIONS].items():
            if content:
                content = self.__fill_collection(content)
            self.__collections.append(CollectionsOptions(coll_name, content, self))

    def __expand_macros_collections(self, parent: IOptions) -> None:
        for collection in self.__collections:
            for coll_param, coll_value in collection.parameters.items():
                try:
                    new_coll_value = Macros.correct_macro(coll_value, self)
                except ParameterNotFound:
                    new_coll_value = Macros.correct_macro(coll_value, parent)

                if new_coll_value != coll_value:
                    collection.parameters[coll_param] = new_coll_value

    def __process_parameters(self, args: dict[str, Any]) -> dict[str, Any]:  # type: ignore[explicit-any]
        result: dict[str, Any] = {}  # type: ignore[explicit-any]
        for param_name, param_value in args.items():
            if param_name.startswith(f"{self.__name}.{self.__PARAMETERS}."):
                param_name = convert_underscore(param_name[(len(self.__name) + len(self.__PARAMETERS) + 2):])
                result[param_name] = param_value
        return result

    def __expand_macros_in_parameters(self) -> None:
        if self.__list_root is not None:
            self.__list_root = Macros.expand_macros_in_path(self.__list_root, self)
        if self.__test_root is not None:
            self.__test_root = Macros.expand_macros_in_path(self.__test_root, self)
        for param_name, param_value in self.__parameters.items():
            if isinstance(param_value, str):
                self.__parameters[param_name] = Macros.correct_macro(param_value, self)

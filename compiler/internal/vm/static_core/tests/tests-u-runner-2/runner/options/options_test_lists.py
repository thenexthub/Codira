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
from typing import Any, cast

from runner.enum_types.configuration_kind import ArchitectureKind, BuildTypeKind, OSKind, SanitizerKind
from runner.options.options import IOptions

PARAMETERS = "parameters"
TEST_LIST = "test-list"
TEST_FILE = "test-file"
SKIP_TEST_LISTS = "skip-test-lists"
EXCLUDE_IGNORED_TESTS = "exclude-ignored-tests"
EXCLUDE_IGNORED_TEST_LISTS = "exclude-ignored-test-lists"
UPDATE_EXCLUDED = "update-excluded"
UPDATE_EXPECTED = "update-expected"
TEST_LIST_ARCH = "test-list-arch"
TEST_LIST_SAN = "test-list-san"
TEST_LIST_OS = "test-list-os"
TEST_LIST_BUILD = "test-list-build"


class TestListsOptions(IOptions):
    DEFAULT_ARCH = ArchitectureKind.AMD64
    DEFAULT_SAN = SanitizerKind.NONE
    DEFAULT_OS = OSKind.LIN
    DEFAULT_BUILD_TYPE = BuildTypeKind.FAST_VERIFY

    def __init__(self, parameters: dict[str, Any]):  # type: ignore[explicit-any]
        super().__init__(parameters)
        self.__parameters = parameters

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        TestListsOptions.add_test_list_cli_args(parser, dest)
        TestListsOptions.add_tags_cli_args(parser, dest)

    @staticmethod
    def add_test_list_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        # Test lists options
        parser.add_argument(
            f'--{TEST_LIST}', default=None,
            dest=f"{dest}{TEST_LIST}",
            help='run only the tests listed in this file')
        parser.add_argument(
            f'--{TEST_FILE}', default=None,
            dest=f"{dest}{TEST_FILE}",
            help='run only one test specified here')
        parser.add_argument(
            f'--{SKIP_TEST_LISTS}', action='store_true', default=False,
            dest=f"{dest}{SKIP_TEST_LISTS}",
            help='do not use ignored or excluded lists, run all available tests, report all found failures')
        ignore_lists_group = parser.add_mutually_exclusive_group()
        ignore_lists_group.add_argument(
            f'--{EXCLUDE_IGNORED_TESTS}', action='store_true', default=False,
            dest=f"{dest}{EXCLUDE_IGNORED_TESTS}",
            help='Consider all ignored test lists as excluded ones')
        ignore_lists_group.add_argument(f'--{EXCLUDE_IGNORED_TEST_LISTS}', action='append', default=None,
                                        type=str, dest=f'{dest}{EXCLUDE_IGNORED_TEST_LISTS}',
                                        help='Consider certain ignored test lists as excluded ones')
        parser.add_argument(
            f'--{UPDATE_EXCLUDED}', action='store_true', default=False,
            dest=f"{dest}{UPDATE_EXCLUDED}",
            help='update list of excluded tests - put all failed tests into default excluded test list')
        parser.add_argument(
            f'--{UPDATE_EXPECTED}', action='store_true', default=False,
            dest=f"{dest}{UPDATE_EXPECTED}",
            help='update files with expected results')

    @staticmethod
    def add_tags_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        # Test lists options
        parser.add_argument(
            f'--{TEST_LIST_ARCH}', action='store',
            default=TestListsOptions.DEFAULT_ARCH,
            type=lambda arg: ArchitectureKind.is_value(arg, f"--{TEST_LIST_ARCH}"),
            dest=f"{dest}{TEST_LIST_ARCH}",
            help='load chosen architecture specific test lists. '
                 f'One of: {ArchitectureKind.values()}')
        parser.add_argument(
            f'--{TEST_LIST_SAN}', action='store',
            default=TestListsOptions.DEFAULT_SAN,
            type=lambda arg: SanitizerKind.is_value(arg, f"--{TEST_LIST_SAN}"),
            dest=f"{dest}{TEST_LIST_SAN}",
            help='load chosen sanitizer specific test lists. '
                 f'One of {SanitizerKind.values()} where '
                 'asan - used on running against build with ASAN and UBSAN sanitizers), '
                 'tsan - used on running against build with TSAN sanitizers).')
        parser.add_argument(
            f'--{TEST_LIST_OS}', action='store',
            default=TestListsOptions.DEFAULT_OS,
            type=lambda arg: OSKind.is_value(arg, f"--{TEST_LIST_OS}"),
            dest=f"{dest}{TEST_LIST_OS}",
            help='load specified operating system specific test lists. '
                 f'One of {OSKind.values()}')
        parser.add_argument(
            f'--{TEST_LIST_BUILD}', action='store',
            default=TestListsOptions.DEFAULT_BUILD_TYPE,
            type=lambda arg: BuildTypeKind.is_value(arg, f"--{TEST_LIST_BUILD}"),
            dest=f"{dest}{TEST_LIST_BUILD}",
            help='load specified build type specific test lists. '
                 f'One of {BuildTypeKind.values()}')

    @cached_property
    def architecture(self) -> ArchitectureKind:
        if isinstance(self.__parameters[TEST_LIST_ARCH], str):
            self.__parameters[TEST_LIST_ARCH] = ArchitectureKind.is_value(
                value=self.__parameters[TEST_LIST_ARCH],
                option_name=f"--{TEST_LIST_ARCH}")
        return cast(ArchitectureKind, self.__parameters[TEST_LIST_ARCH])

    @cached_property
    def build_type(self) -> BuildTypeKind:
        if isinstance(self.__parameters[TEST_LIST_BUILD], str):
            self.__parameters[TEST_LIST_BUILD] = BuildTypeKind.is_value(
                value=self.__parameters[TEST_LIST_BUILD],
                option_name=f"--{TEST_LIST_BUILD}")
        return cast(BuildTypeKind, self.__parameters[TEST_LIST_BUILD])

    @cached_property
    def explicit_file(self) -> str | None:
        value = self.__parameters[TEST_FILE]
        return str(value) if value is not None else value

    @cached_property
    def explicit_list(self) -> str | None:
        value = self.__parameters[TEST_LIST]
        return str(value) if value is not None else value

    @cached_property
    def skip_test_lists(self) -> bool:
        return cast(bool, self.__parameters[SKIP_TEST_LISTS])

    @cached_property
    def exclude_ignored_tests(self) -> str | set[str] | None:
        return cast(str | set[str] | None, self.__parameters[EXCLUDE_IGNORED_TESTS])

    @cached_property
    def exclude_ignored_test_lists(self) -> set[str] | None:
        return cast(set[str] | None, self.__parameters[EXCLUDE_IGNORED_TEST_LISTS])

    @cached_property
    def update_excluded(self) -> bool:
        return cast(bool, self.__parameters[UPDATE_EXCLUDED])

    @cached_property
    def update_expected(self) -> bool:
        return cast(bool, self.__parameters[UPDATE_EXPECTED])

    def get_command_line(self) -> str:
        options = [
            f'--test-file="{self.explicit_file}"' if self.explicit_file is not None else '',
            f'--test-list="{self.explicit_list}"' if self.explicit_list is not None else '',
            '--skip-test-lists' if self.skip_test_lists else '',
            '--exclude-ignored-tests' if self.exclude_ignored_tests else '',
            '--update-excluded' if self.update_excluded else '',
            '--update-expected' if self.update_expected else ''
        ]
        return ' '.join(options)

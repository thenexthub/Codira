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
from typing import Any

from runner.options.options import IOptions
from runner.utils import check_int, is_file


class GroupsOptions(IOptions):
    __PARAMETERS = "parameters"
    __GROUPS = "groups"
    __DEFAULT_GROUPS = 1
    __GROUP_NUMBER = "group-number"
    __DEFAULT_GROUP_NUMBER = 1
    __CHAPTERS = "chapters"
    __CHAPTERS_FILE = "chapters-file"
    __DEFAULT_CHAPTERS_FILE = "chapters.yaml"
    __CHAPTERS_DELIMITER = ";"

    def __init__(self, parameters: dict[str, Any]):  # type: ignore[explicit-any]
        super().__init__()
        self.__parameters = parameters

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        # Test groups options
        test_groups = parser.add_argument_group(
            "Test groups",
            "allowing to divide tests into groups and run groups separately")
        test_groups.add_argument(
            f'--{GroupsOptions.__GROUPS}', action='store',
            type=lambda arg: check_int(arg, f"--{GroupsOptions.__GROUPS}", is_zero_allowed=False),
            default=GroupsOptions.__DEFAULT_GROUPS,
            dest=f"{dest}{GroupsOptions.__GROUPS}",
            help='Quantity of groups used for automatic division. '
                 f'By default {GroupsOptions.__DEFAULT_GROUPS} - all tests in one group')
        test_groups.add_argument(
            f'--{GroupsOptions.__GROUP_NUMBER}', action='store',
            type=lambda arg: check_int(arg, f"--{GroupsOptions.__GROUP_NUMBER}", is_zero_allowed=False),
            default=GroupsOptions.__DEFAULT_GROUP_NUMBER,
            dest=f"{dest}{GroupsOptions.__GROUP_NUMBER}",
            help='run tests only of specified group number. Used only if --groups is set. '
                 f'Default value {GroupsOptions.__DEFAULT_GROUP_NUMBER} - the first available group. '
                 'If the value is more than the total number of groups the latest group is taken.')
        test_groups.add_argument(
            f'--{GroupsOptions.__CHAPTERS}', action='store',
            type=str, default=None,
            dest=f"{dest}{GroupsOptions.__CHAPTERS}",
            help='run tests only of specified chapters. '
                 f'Divide several chapters with `{GroupsOptions.__CHAPTERS_DELIMITER}`')
        test_groups.add_argument(
            f'--{GroupsOptions.__CHAPTERS_FILE}', action='store',
            type=lambda arg: is_file(arg) if arg != GroupsOptions.__DEFAULT_CHAPTERS_FILE else arg,
            default=GroupsOptions.__DEFAULT_CHAPTERS_FILE,
            dest=f"{dest}{GroupsOptions.__CHAPTERS_FILE}",
            help='Full name to the file with chapter description. '
                 f'By default {GroupsOptions.__DEFAULT_CHAPTERS_FILE} file located along with test lists.')

    @cached_property
    def quantity(self) -> int:
        value = self.__parameters[self.__GROUPS]
        return value if isinstance(value, int) else int(value)

    @cached_property
    def number(self) -> int:
        value = self.__parameters[self.__GROUP_NUMBER]
        return value if isinstance(value, int) else int(value)

    @cached_property
    def chapters(self) -> list[str]:
        chapters = self.__parameters[self.__CHAPTERS]
        if chapters is None:
            return []
        return list(chapters.split(GroupsOptions.__CHAPTERS_DELIMITER))

    @cached_property
    def chapters_file(self) -> Path:
        return Path(self.__parameters[self.__CHAPTERS_FILE])

    def get_command_line(self) -> str:
        options = [
            f'--group={self.quantity}' if self.quantity != GroupsOptions.__DEFAULT_GROUPS else '',
            f'--group-number={self.number}' if self.number != GroupsOptions.__DEFAULT_GROUP_NUMBER else ''
        ]
        return ' '.join(options)

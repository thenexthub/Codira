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
from typing import Any, ClassVar

from runner.options.options import IOptions


class TimeReportOptions(IOptions):
    __DEFAULT_EDGES: ClassVar[list[int]] = [1, 5, 10]
    __TIME_REPORT = "enable-time-report"
    __TIME_EDGES = "time-edges"

    def __init__(self, args: dict[str, Any]):  # type: ignore[explicit-any]
        super().__init__(args)
        self.__parameters = args

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        parser.add_argument(
            f'--{TimeReportOptions.__TIME_REPORT}', action='store_true', default=False,
            dest=f"{dest}{TimeReportOptions.__TIME_REPORT}",
            help='Log execution test time')
        parser.add_argument(
            f'--{TimeReportOptions.__TIME_EDGES}', action='store',
            default=TimeReportOptions.__DEFAULT_EDGES,
            type=TimeReportOptions.__process_time_edges,
            dest=f"{dest}{TimeReportOptions.__TIME_EDGES}",
            help='Time edges in the format `1,5,10` where numbers are seconds')

    @staticmethod
    def __process_time_edges(arg: str) -> list[int]:
        return [int(v) for v in arg.split(",")]

    @cached_property
    def enable(self) -> bool:
        return bool(self.__parameters[self.__TIME_REPORT])

    @cached_property
    def time_edges(self) -> list[int]:
        return list(self.__parameters[self.__TIME_EDGES])

    def get_command_line(self) -> str:
        _edges: str = ','.join(str(self.time_edges))
        options: list[str] = [
        ]
        return ' '.join(options)

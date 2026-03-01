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
import enum

from runner.options.options import IOptions


class ReportOptionsDefaults(enum.Enum):
    DETAILED_REPORT = "detailed-report"
    DETAILED_REPORT_FILE = "detailed-report-file"
    REPORT_DIR = "report-dir"
    DEFAULT_DETAILED_REPORT = False
    DEFAULT_DETAILED_REPORT_FILE = "detailed-report-file"
    DEFAULT_REPORT_DIR = "report"


class ReportOptions(IOptions):

    def __init__(self, args: dict[str, str | bool]):
        super().__init__(args)

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_report_cli(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        report_config = parser.add_argument_group(title="URunner report options")
        report_config.add_argument(
            f'--{ReportOptionsDefaults.DETAILED_REPORT.value}', action='store_true',
            default=ReportOptionsDefaults.DEFAULT_DETAILED_REPORT.value,
            dest=f"{dest}{ReportOptionsDefaults.DETAILED_REPORT.value}",
            help='Create additional detailed report with counting tests for each folder.')
        report_config.add_argument(
            f'--{ReportOptionsDefaults.DETAILED_REPORT_FILE.value}', action='store',
            default=ReportOptionsDefaults.DEFAULT_DETAILED_REPORT_FILE.value,
            dest=f"{dest}{ReportOptionsDefaults.DETAILED_REPORT_FILE.value}",
            help='Name of additional detailed report. By default, the report is created at '
                 '$WorkDir/<suite-name>/report/<suite-name>_detailed-report-file.md , '
                 'where $WorkDir is the folder specified by the environment variable WORK_DIR')
        report_config.add_argument(
            f'--{ReportOptionsDefaults.REPORT_DIR.value}', action='store',
            default=ReportOptionsDefaults.DEFAULT_REPORT_DIR.value,
            dest=f"{dest}{ReportOptionsDefaults.REPORT_DIR.value}",
            help='Name of report folder under $WorkDir. By default, the name is "report".'
                 'The location is "$WorkDir/<suite-name>/<report-dir>", '
                 'where $WorkDir is the folder specified by the environment variable WORK_DIR')

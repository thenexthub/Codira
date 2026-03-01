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

from abc import ABC, abstractmethod
from collections.abc import Mapping
from os import path
from typing import TYPE_CHECKING, TypeAlias

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.params import TestEnv, TestReport
from runner.enum_types.verbose_format import VerboseFilter
from runner.logger import Log
from runner.reports.report_format import ReportFormat
from runner.utils import write_2_file

if TYPE_CHECKING:
    from runner.test_base import GTest, Test
    TestType: TypeAlias = GTest | Test

_LOGGER = Log.get_logger(__file__)


class ReportGenerator:
    def __init__(self, test_id: str, test_env: TestEnv, repeat: int) -> None:
        self.__test_id = test_id
        self.__test_env = test_env
        self.__repeat = repeat
        self.__generate_for_passed = test_env.config.general.verbose_filter == VerboseFilter.ALL

    def generate_reports(self, test_result: 'TestType') -> dict[ReportFormat, str]:
        if test_result.passed and not self.__generate_for_passed:
            return {}

        reports_format_to_paths = {}
        format_to_report: Mapping[ReportFormat, type[Report]] = {
            ReportFormat.HTML: HtmlReport,
            ReportFormat.MD: MdReport,
            ReportFormat.LOG: TextReport
        }

        report_tail = "passed" if test_result.passed else "known" if test_result.ignored else "new"
        report_root = path.join(self.__test_env.work_dir.report, report_tail)
        for report_format in list(self.__test_env.report_formats):
            repeat_str = f"-{self.__repeat:05d}"
            report_path = path.join(
                report_root,
                f"{test_result.test_id}.report{repeat_str}-{self.__test_env.timestamp}.{report_format.value}")
            if (formatter := format_to_report.get(report_format)) is None:
                raise InvalidConfiguration(f"Incorrect report format {report_format}")
            report = formatter(test_result)
            _LOGGER.all(f"{self.__test_id}: Report is saved to {report_path}")
            write_2_file(report_path, report.make_report())
            reports_format_to_paths[report_format] = report_path

        return reports_format_to_paths


REPORT_TITLE = "${Title}"
REPORT_PATH = "${Path}"
REPORT_STATUS_CLASS = "${status_class}"
REPORT_STATUS = "${Status}"
REPORT_REPRODUCE = "${Reproduce}"
REPORT_RESULT = "${Result}"
REPORT_EXPECTED = "${Expected}"
REPORT_ACTUAL = "${Actual}"
REPORT_ERROR = "${Error}"
REPORT_RETURN_CODE = "${ReturnCode}"
REPORT_TIME = "${Time}"

STATUS_PASSED = "PASSED"
STATUS_PASSED_CLASS = "test_status--passed"
STATUS_FAILED = "FAILED"
STATUS_FAILED_CLASS = "test_status--failed"

NO_TIME = "not measured"


def convert_to_array(output: str) -> list[str]:
    return [line.strip() for line in output.split("\n") if len(line.strip()) > 0]


class Report(ABC):
    def __init__(self, test: 'Test') -> None:
        self.test = test

    @abstractmethod
    def make_report(self) -> str:
        pass


class HtmlReport(Report):
    @staticmethod
    def __get_good_line(line: str) -> str:
        return f'<span class="output_line">{line}</span>'

    @staticmethod
    def __get_failed_line(line: str) -> str:
        return f'<span class="output_line output_line--failed">{line}</span>'

    def make_report(self) -> str:
        actual_report = self.test.report if self.test.report is not None else TestReport("", "", -1)
        expected, actual = self.__make_output_diff_html(self.test.expected, actual_report.output)
        test_expected, test_actual = "\n".join(expected), "\n".join(actual)

        report_path = path.join(path.dirname(path.abspath(__file__)), "report_template.html")
        with open(report_path, encoding="utf-8") as file_pointer:
            report = file_pointer.read()

        report = report.replace(REPORT_TITLE, self.test.test_id)
        report = report.replace(REPORT_PATH, str(self.test.path))
        if self.test.passed:
            report = report.replace(REPORT_STATUS_CLASS, STATUS_PASSED_CLASS)
            report = report.replace(REPORT_STATUS, STATUS_PASSED)
        else:
            report = report.replace(REPORT_STATUS_CLASS, STATUS_FAILED_CLASS)
            report = report.replace(REPORT_STATUS, STATUS_FAILED)
        if self.test.time is not None:
            report = report.replace(REPORT_TIME, f"{round(self.test.time, 2)} sec")
        else:
            report = report.replace(REPORT_TIME, NO_TIME)

        report = report.replace(REPORT_REPRODUCE, self.test.reproduce)
        report = report.replace(REPORT_EXPECTED, test_expected)
        report = report.replace(REPORT_ACTUAL, test_actual)
        report = report.replace(REPORT_ERROR, actual_report.error)
        if self.test.report is None:
            report = report.replace(REPORT_RETURN_CODE, "Not defined")
        else:
            report = report.replace(REPORT_RETURN_CODE, str(actual_report.return_code))

        return report

    def __make_output_diff_html(self, expected: str, actual: str) -> tuple[list[str], list[str]]:
        expected_list = convert_to_array(expected)
        actual_list = convert_to_array(actual)
        result_expected = []
        result_actual = []

        min_len = min(len(expected_list), len(actual_list))
        for i in range(min_len):
            expected_line = expected_list[i].strip()
            actual_line = actual_list[i].strip()
            if expected_line == actual_line:
                result_expected.append(self.__get_good_line(expected_line))
                result_actual.append(self.__get_good_line(actual_line))
            else:
                result_expected.append(self.__get_failed_line(expected_line))
                result_actual.append(self.__get_failed_line(actual_line))

        max_len = max(len(expected_list), len(actual_list))
        is_expected_remains = len(expected_list) > len(actual_list)
        for i in range(min_len, max_len):
            if is_expected_remains:
                result_expected.append(self.__get_good_line(expected_list[i]))
            else:
                result_actual.append(self.__get_good_line(actual_list[i]))

        return result_expected, result_actual


class MdReport(Report):
    @staticmethod
    def __get_md_good_line(expected: str, actual: str) -> str:
        return f"| {expected} | {actual} |"

    @staticmethod
    def __get_md_failed_line(expected: str, actual: str) -> str:
        if expected.strip() != "":
            expected = f"**{expected}**"
        if actual.strip() != "":
            actual = f"**{actual}**"
        return f"| {expected} | {actual} |"

    def make_report(self) -> str:
        actual_report = self.test.report if self.test.report is not None else TestReport("", "", -1)
        result = self.__make_output_diff_md(self.test.expected, actual_report.output)
        test_result = "\n".join(result)

        report_path = path.join(path.dirname(path.abspath(__file__)), "report_template.md")
        with open(report_path, encoding="utf-8") as file_pointer:
            report = file_pointer.read()

        report = report.replace(REPORT_TITLE, self.test.test_id)
        report = report.replace(REPORT_PATH, str(self.test.path))
        if self.test.passed:
            report = report.replace(REPORT_STATUS_CLASS, STATUS_PASSED_CLASS)
            report = report.replace(REPORT_STATUS, STATUS_PASSED)
        else:
            report = report.replace(REPORT_STATUS_CLASS, STATUS_FAILED_CLASS)
            report = report.replace(REPORT_STATUS, STATUS_FAILED)
        if self.test.time is not None:
            report = report.replace(REPORT_TIME, f"{round(self.test.time, 2)} sec")
        else:
            report = report.replace(REPORT_TIME, NO_TIME)

        report = report.replace(REPORT_REPRODUCE, self.test.reproduce)
        report = report.replace(REPORT_RESULT, test_result)
        report = report.replace(REPORT_ERROR, actual_report.error)
        if self.test.report is None:
            report = report.replace(REPORT_RETURN_CODE, "Not defined")
        else:
            report = report.replace(REPORT_RETURN_CODE, str(actual_report.return_code))

        return report

    def __make_output_diff_md(self, expected: str, actual: str) -> list[str]:
        expected_list = convert_to_array(expected)
        actual_list = convert_to_array(actual)
        result = []

        min_len = min(len(expected_list), len(actual_list))
        for i in range(min_len):
            expected_line = expected_list[i].strip()
            actual_line = actual_list[i].strip()
            if expected_line == actual_line:
                result.append(self.__get_md_good_line(expected_line, actual_line))
            else:
                result.append(self.__get_md_failed_line(expected_line, actual_line))

        max_len = max(len(expected_list), len(actual_list))
        is_expected_remains = len(expected_list) > len(actual_list)
        for i in range(min_len, max_len):
            if is_expected_remains:
                result.append(self.__get_md_good_line(expected_list[i], " "))
            else:
                result.append(self.__get_md_failed_line(" ", actual_list[i]))

        return result


class TextReport(Report):
    def make_report(self) -> str:
        result = "PASSED" if self.test.passed else "FAILED"
        time_line = f"{round(self.test.time, 2)} sec" if self.test.time is not None else NO_TIME
        return "\n".join([
            f"Test ID: {self.test.test_id}",
            f"Test path: {self.test.path}\n",
            f"Result: {result}",
            f"Execution time: {time_line}\n",
            f"Steps to reproduce:{self.test.reproduce}\n",
        ])

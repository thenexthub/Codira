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
from datetime import date
from os import path
from pathlib import Path

from runner.logger import Log
from runner.reports.summary import Summary
from runner.test_base import Test
from runner.utils import write_2_file

_LOGGER = Log.get_logger(__file__)


class DetailedReport:
    REPORT_DATE = "${Date}"
    REPORT_TEST_SUITE = "${TestSuite}"
    REPORT_EXCLUDED_HEADER = "${ExcludedHeader}"
    REPORT_EXCLUDED_DIVIDER = "${ExcludedDivider}"
    REPORT_RESULT = "${Result}"

    TEMPLATE = "detailed_report_template.md"
    EXCLUDED_HEADER = " Excluded after |"
    EXCLUDED_DIVIDER = "-------|"

    def __init__(self, results: list[Test], test_suite: str, report_path: Path, report_file: Path | None):
        self.tests: list[Test] = results
        self.result: dict[str, Summary] = {}
        self.test_suite = test_suite
        self.has_excluded = False
        if report_file is None:
            self.report_file = path.join(report_path, f"{test_suite}_detailed-report.md")
        else:
            self.report_file = str(report_file)
        self.__calculate()

    @staticmethod
    def __get_paths(test_id: str) -> list[str]:
        """
        :param test_id: test_id or file path delimited by '/'
        :return: list of iterative partial parts
        Example: test_id = "a/b/c/d"
        return value: ["a/", "a/b/", "a/b/c/"]
        """
        parts = test_id.split("/")[:-1]
        if len(parts) < 2:
            return parts
        partial_path = ''
        paths: list[str] = []
        for part in parts:
            partial_path += part + '/'
            paths.append(partial_path)

        return paths

    def populate_report(self) -> None:
        template_path = path.join(path.dirname(path.abspath(__file__)), self.TEMPLATE)
        with open(template_path, encoding="utf-8") as file_pointer:
            report = file_pointer.read()
        report = report.replace(self.REPORT_DATE, str(date.today()))
        report = report.replace(self.REPORT_TEST_SUITE, self.test_suite)
        if not self.has_excluded:
            report = report.replace(self.REPORT_EXCLUDED_HEADER, "")
            report = report.replace(self.REPORT_EXCLUDED_DIVIDER, "")
        else:
            report = report.replace(self.REPORT_EXCLUDED_HEADER, self.EXCLUDED_HEADER)
            report = report.replace(self.REPORT_EXCLUDED_DIVIDER, self.EXCLUDED_DIVIDER)
        lines: list[str] = []
        for partial_path, summary in self.result.items():
            lines.append(self.__report_one_line(partial_path, summary))
        result = "\n".join(sorted(lines))
        report = report.replace(self.REPORT_RESULT, result)
        _LOGGER.default(f"Detailed report is saved to '{self.report_file}'")
        write_2_file(self.report_file, report)

    def __calculate_one_test(self, test: Test) -> None:
        test_paths = self.__get_paths(test.test_id)
        is_passed_not_ignored = test.passed and not test.ignored
        is_passed_and_ignored = test.passed and test.ignored
        is_failed_not_ignored = not test.passed and not test.ignored
        is_failed_and_ignored = not test.passed and test.ignored
        is_excluded_after = test.excluded
        for test_path in test_paths:
            if test_path not in self.result:
                self.result[test_path] = Summary(test_path)
            summary = self.result[test_path]
            summary.total += 1
            if is_excluded_after:
                summary.excluded_after += 1
                self.has_excluded = True
            elif is_passed_not_ignored:
                summary.passed += 1
            elif is_failed_not_ignored:
                summary.failed += 1
            elif is_passed_and_ignored:
                summary.ignored_but_passed += 1
            elif is_failed_and_ignored:
                summary.ignored += 1
            self.result[test_path] = summary

    def __calculate(self) -> None:
        for test in self.tests:
            self.__calculate_one_test(test)

    def __report_one_line(self, folder: str, summary: Summary) -> str:
        report = (f"| {folder} | {summary.total} | "
                  f"{summary.passed} | {summary.failed} | "
                  f"{summary.ignored_but_passed} | {summary.ignored} |")
        if self.has_excluded:
            report += f" {summary.excluded_after} |"
        return report

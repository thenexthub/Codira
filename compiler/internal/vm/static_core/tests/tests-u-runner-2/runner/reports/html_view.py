#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
from datetime import datetime
from os import path
from pathlib import Path

import pytz

from runner.options.config import Config
from runner.reports.report_format import ReportFormat
from runner.reports.summary import Summary
from runner.test_base import Test
from runner.utils import write_2_file

INDEX_TITLE = "${Title}"
INDEX_OPTIONS = "${Options}"
INDEX_TOTAL = "${Total}"
INDEX_PASSED = "${Passed}"
INDEX_FAILED = "${Failed}"
INDEX_IGNORED = "${Ignored}"
INDEX_EXCLUDED_LISTS = "${ExcludedThroughLists}"
INDEX_EXCLUDED_OTHER = "${ExcludedByOtherReasons}"
INDEX_TEST_NAME = "${TestName}"
INDEX_TEST_ID = "${TestId}"
INDEX_FAILED_TESTS_LIST = "${FailedTestsList}"


class HtmlView:
    def __init__(self, report_root: Path, config: Config, summary: Summary) -> None:
        self.__report_root = report_root
        self.__config = config
        self.__summary = summary

    def create_html_index(self, fail_lists: dict[str, list[Test]], timestamp: int) -> None:
        report_file = path.join(path.dirname(path.abspath(__file__)), "index_template.html")
        with open(report_file, encoding="utf-8") as file_pointer:
            report = file_pointer.read()

        report = report.replace(INDEX_TITLE, f"Summary for {self.__summary.name} {datetime.now(pytz.UTC)}")
        report = report.replace(INDEX_OPTIONS, str(self.__config))
        report = report.replace(INDEX_TOTAL, str(self.__summary.total))
        report = report.replace(INDEX_PASSED, str(self.__summary.passed))
        report = report.replace(INDEX_FAILED, str(self.__summary.failed))
        report = report.replace(INDEX_IGNORED, str(self.__summary.ignored))
        report = report.replace(INDEX_EXCLUDED_LISTS, str(self.__summary.excluded))
        report = report.replace(INDEX_EXCLUDED_OTHER, str(self.__summary.excluded_after))
        report = report.replace(INDEX_FAILED_TESTS_LIST, self.__get_failed_tests_report(fail_lists))

        report_path = self.__report_root / f"{self.__summary.name}.report-{timestamp}.html"
        write_2_file(report_path, report)

    def __get_failed_tests_report(self, fail_lists: dict[str, list[Test]]) -> str:
        new = "new"
        failed_tests: list[tuple[str, str]] = []
        for kind in fail_lists:
            for test in fail_lists[kind]:
                report_path = test.reports[ReportFormat.HTML][len(str(self.__report_root)) + 1:]
                failed_tests.append((test.test_id, report_path))
        failed_tests.sort(key=lambda x: "1" + x[1] if x[1].startswith(new) else x[1])

        line_template_new = '<li class="link-container">' \
                            '<b>NEW</b> ' \
                            '<a href="${TestName}" class="link">${TestId}</a>' \
                            '</li>'
        line_template_known = '<li class="link-container">' \
                              '<a href="${TestName}" class="link">${TestId}</a>' \
                              '</li>'

        failed_tests_report = []
        for failed in failed_tests:
            failed_id, failed_path = failed
            template = line_template_new if failed_path.startswith(new) else line_template_known
            replaced = template.replace(INDEX_TEST_NAME, failed_path).replace(INDEX_TEST_ID, failed_id)
            failed_tests_report.append(replaced)
        return "\n".join(failed_tests_report)

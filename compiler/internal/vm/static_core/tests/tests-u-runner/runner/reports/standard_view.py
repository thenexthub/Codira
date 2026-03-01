#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#
#

import logging
from datetime import datetime
from pathlib import Path
from typing import List, Dict

import pytz
from runner.logger import Log
from runner.options.options_time_report import TimeReportOptions
from runner.reports.report_format import ReportFormat
from runner.reports.summary import Summary
from runner.enum_types.fail_kind import FailKind
from runner.utils import write_2_file
from runner.test_base import Test

_LOGGER = logging.getLogger("runner.reports.standard_view")


class StandardView:
    def __init__(self, report_root: Path, update_excluded: bool, excluded_lists: List[str], summary: Summary) -> None:
        self.__report_root = report_root
        # filename of text report of new failures along with path to log
        self.__test_report_fails = 'failures.txt'
        self.__update_excluded = update_excluded
        self.__excluded_lists = excluded_lists
        self.__summary = summary

    @staticmethod
    def summarize_failures(fail_lists: Dict[FailKind, List[Test]]) -> None:
        for kind, fail_list in fail_lists.items():
            if len(fail_list) == 0:
                continue
            fail_list.sort(key=lambda t: t.path)
            content: str = "\n".join([str(test.test_id) for test in fail_list])
            Log.default(_LOGGER, f"New failures at #{kind} - {len(fail_list)} tests:\n{content}")
            Log.all(_LOGGER, "")

    @staticmethod
    def summarize_passed_ignored(ignored_but_passed: List[Test]) -> None:
        ignored_but_passed.sort(key=lambda t: t.path)
        content: str = "\n".join([str(test.test_id) for test in ignored_but_passed])
        Log.all(_LOGGER, f"Ignored but passed tests - {len(ignored_but_passed)} tests:\n{content}")

    def display_summary(self, fail_lists: Dict[FailKind, List[Test]]) -> None:
        Log.default(_LOGGER, f"Summary({self.__summary.name}):")
        Log.default(_LOGGER, f"Total:                       \t{self.__summary.total:<5}")
        Log.default(_LOGGER, f"Passed:                      \t{self.__summary.passed:<5}")
        Log.default(_LOGGER, f"Failed (new failures):       \t{self.__summary.failed:<5}")
        for kind, fail_list in fail_lists.items():
            if len(fail_list) == 0:
                continue
            Log.default(_LOGGER, f"  {kind}:" + ' ' * (29 - len(str(kind))) + f"{len(fail_list):<5}")
        Log.default(_LOGGER, f"Ignored, but passed:         \t{self.__summary.ignored_but_passed:<5}")
        Log.default(_LOGGER, f"Ignored (known failures):    \t{self.__summary.ignored:<5}")
        if self.__summary.excluded > 0:
            Log.default(_LOGGER, f"Excluded through lists:      \t{self.__summary.excluded:<5}")
        if self.__summary.excluded_after > 0:
            Log.default(_LOGGER, f"Excluded by other reasons:   \t{self.__summary.excluded_after:<5}")

    def create_failures_report(self, fail_lists: Dict[FailKind, List[Test]]) -> None:
        flat_failed_tests = []
        for kind in fail_lists:
            for test in fail_lists[kind]:
                report_path = test.reports[ReportFormat.MD] \
                    if ReportFormat.MD in test.reports \
                    else test.reports[ReportFormat.LOG]
                flat_failed_tests.append((test.test_id, report_path))
        if not flat_failed_tests:
            return
        new_failures_path = self.__report_root / self.__test_report_fails
        Log.all(_LOGGER, f"Save list of new failures to {new_failures_path}")
        write_2_file(new_failures_path, '\n'.join(' '.join(x) for x in flat_failed_tests))

    def create_update_excluded(self, excluded_but_passed: List[Test], excluded_still_failed: List[Test]) -> None:
        if not self.__update_excluded:
            return
        Log.default(_LOGGER, "Update excluded tests:")
        Log.default(_LOGGER, f"Passed: {len(excluded_but_passed)} - removed from excluded list")
        Log.default(_LOGGER, f"Still failed: {len(excluded_still_failed)}")

        self.__create_updated(excluded_still_failed)
        self.__create_passed(excluded_but_passed)

    def create_time_report(self, results: List[Test], time_report_options: TimeReportOptions) -> None:
        if not time_report_options.enable:
            return
        time_edges: List[int] = time_report_options.time_edges[:]
        tests_by_time: List[List[str]] = []
        for _ in range(len(time_edges) + 1):
            tests_by_time.append([])

        for test_result in results:
            if test_result.time is None:
                continue
            for i, time_edge in enumerate(time_edges):
                if test_result.time < time_edge:
                    tests_by_time[i].append(test_result.test_id)
                    break
            else:
                tests_by_time[-1].append(f"{test_result.test_id} # {round(test_result.time, 2)} sec")

        Log.summary(_LOGGER, "Test execution time")
        time_report = ""
        for i, time_edge in enumerate(time_edges):
            Log.summary(_LOGGER, f"Less {time_edge} sec: {len(tests_by_time[i])}")
            time_report += f"\nLess {time_edge} sec:\n"
            for tests_for_time in tests_by_time[i]:
                time_report += f"{tests_for_time}\n"
        Log.summary(_LOGGER, f"More {time_edges[-1]} sec: {len(tests_by_time[-1])}")
        time_report += f"\n{time_edges[-1]} sec or more:\n"
        for tests_for_time in tests_by_time[-1]:
            time_report += f"{tests_for_time}\n"

        timestamp = int(datetime.timestamp(datetime.now(pytz.UTC)))
        time_report_path = self.__report_root / f"{self.__summary.name}-time_report-{timestamp}.txt"

        write_2_file(time_report_path, time_report)
        Log.all(_LOGGER, f"Time report saved to {time_report_path}")

    def __create_updated(self, tests: List[Test]) -> None:
        name, state = ("updated", "still failed")
        self.__create_file(tests, name, state)

    def __create_passed(self, tests: List[Test]) -> None:
        name, state = ("passed",) * 2
        self.__create_file(tests, name, state)

    def __create_file(self, tests: List[Test], name: str, state: str) -> None:
        if len(tests) == 0:
            return
        sorted_test_ids: List[str] = sorted([test.test_id for test in tests])
        new_excl_list_path = self.__excluded_lists[0][:-4] + f"_{name}.txt"
        Log.default(_LOGGER, f"Save list of excluded but {state} tests: {new_excl_list_path}")
        write_2_file(new_excl_list_path, "\n".join(sorted_test_ids))

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

import unittest
from pathlib import Path

from runner.plugins.ets.test_ets import TestETS
from runner.enum_types.params import TestReport


def get_test_instance(report_stdout: str, report_stderr: str, expected_file: str,
                      expected_err_file: str) -> TestETS:
    test_inst = TestETS.__new__(TestETS)
    test_inst.expected = ""
    test_inst.expected_err = ''
    test_inst.passed = True
    test_inst.report = TestReport(report_stdout, report_stderr, 0)
    test_inst.test_expected = Path(__file__).parent / expected_file
    test_inst.test_expected_err = Path(__file__).parent / expected_err_file

    return test_inst


class ExpectedFilesTest(unittest.TestCase):


    def test_compare_output_ok(self) -> None:
        report_stdout = "default: 1\ndefault: 2\ndefault: 3"
        report_stderr = ""
        expected_file = "ConsoleTest1.ets.expected"
        expected_err_file = "ConsoleTest1.ets.expected.err"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        if test_result.report is not None:
            passed, _ = test_result.compare_output_with_expected(test_result.report.output,
                                                                    test_result.report.error)
            self.assertTrue(passed)

    def test_compare_output_but_stderr(self) -> None:
        report_stdout = "default: 1\ndefault: 2\ndefault: 3"
        report_stderr = "error!"
        expected_file = "ConsoleTest1.ets.expected"
        expected_err_file = "ConsoleTest1.ets.expected.err"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        if test_result.report is not None:
            passed, _ = test_result.compare_output_with_expected(test_result.report.output,
                                                                    test_result.report.error)
            self.assertFalse(passed)

    def test_compare_output_fail(self) -> None:
        report_stdout = "default: 2\ndefault: 2\ndefault: 3"
        report_stderr = ""
        expected_file = "ConsoleTest1.ets.expected"
        expected_err_file = "ConsoleTest1.ets.expected.err"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        if test_result.report is not None:
            passed, _ = test_result.compare_output_with_expected(test_result.report.output,
                                                                    test_result.report.error)
            self.assertFalse(passed)

    def test_compare_stderr_ok(self) -> None:
        report_stdout = ""
        report_stderr = "default: 1\ndefault: 2\ndefault: 3"
        expected_file = "ConsoleTest2.ets.expected"
        expected_err_file = "ConsoleTest2.ets.expected.err"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        if test_result.report is not None:
            passed, _ = test_result.compare_output_with_expected(test_result.report.output,
                                                                    test_result.report.error)
            self.assertTrue(passed)

    def test_compare_stderr_fail(self) -> None:
        report_stdout = ""
        report_stderr = "default: 2\ndefault: 2\ndefault: 3"
        expected_file = "ConsoleTest2.ets.expected"
        expected_err_file = "ConsoleTest2.ets.expected.err"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        if test_result.report is not None:
            passed, _ = test_result.compare_output_with_expected(test_result.report.output,
                                                                    test_result.report.error)
            self.assertFalse(passed)

    def test_compare_both(self) -> None:
        report_stdout = "default: 1\ndefault: 2\ndefault: 3"
        report_stderr = "default: 4\ndefault: 5\ndefault: 6"
        expected_file = "ConsoleTest3.ets.expected"
        expected_err_file = "ConsoleTest3.ets.expected.err"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        if test_result.report is not None:
            passed, _ = test_result.compare_output_with_expected(test_result.report.output,
                                                                    test_result.report.error)
            self.assertTrue(passed)

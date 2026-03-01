#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
from pathlib import Path
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner.enum_types.validation_result import ValidatorFailKind
from runner.extensions.validators.parser.parser_validator import ParserValidator
from runner.suites.test_standard_flow import TestStandardFlow


class ParserValidatorTest(TestCase):
    data_folder = Path(__file__).parent / "data"

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_no_output(self, mock_test: MagicMock) -> None:
        """
        Test checks simple passed way: no output, expected file is empty, return code = 0
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test1.ets"
        step_name = "step"
        actual_output = ""
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_absent_expected(self, mock_test: MagicMock) -> None:
        """
        Input: no output, expected file is absent
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test2.ets"
        step_name = "step"
        actual_output = ""
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)
        self.assertEqual(actual.kind, ValidatorFailKind.NONE)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, return code = 0
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test3.ets"
        step_name = "step"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)
        self.assertEqual(actual.kind, ValidatorFailKind.NONE)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_failed_not_matched_output(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains non-matching content.
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test3.ets"
        step_name = "step"
        actual_output = "hello\nword"
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)
        self.assertEqual(actual.kind, ValidatorFailKind.COMPARE_OUTPUT)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output_rt1(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, but return code = 1
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test3.ets"
        step_name = "step"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 1
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output_rt2(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, but return code = 2
        Expected: test failed, as return code is neither 0 nor 1
        """
        mock_test.path = self.data_folder / "test3.ets"
        step_name = "step"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 2
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)

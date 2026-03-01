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
import os
import shutil
from collections import namedtuple
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import patch

from runner.extensions.validators.base_validator import BaseValidator
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.reports.report_format import ReportFormat
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.test import test_utils


class FailuresFromIgnoreTest(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__, "test_data")
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)

    @staticmethod
    def get_config() -> Config:
        args = get_args()
        config = Config(args)
        return config

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite1"],),
        (["runner.sh", "panda", "test_suite1", "--gn-build"],),
    ])
    def test_load_comments_from_ignore_list(self, argv: Callable) -> None:
        """
        Check tests are loading from ignore lists with comments
        """
        with patch('sys.argv', argv):
            ExpectedTest = namedtuple('ExpectedTest', 'test_id ignored last_failure')
            test1 = ExpectedTest("test1.ets", True, "test failure")
            test2 = ExpectedTest("test2.ets", False, "")
            test3 = ExpectedTest("test3.ets", True, "test failed.*?error")
            test4 = ExpectedTest("test4.ets", True, "")
            test5 = ExpectedTest("test5_compile_only_neg.ets", False, "")
            test6 = ExpectedTest("test1.console.ets", False, "")
            expected_tests = [test1, test2, test3, test4, test5, test6]

            config = self.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = list(runner.tests)

            self.assertEqual({(t.test_id, t.ignored, t.last_failure) for t in actual_tests},
                                {(e.test_id, e.ignored, e.last_failure) for e in expected_tests})

            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite1"],),
        (["runner.sh", "panda", "test_suite1", "--gn-build"],),
    ])
    def test_results_with_expected_failure_pass(self, argv: Callable) -> None:
        with patch('sys.argv', argv):
            test_output = "test failure"
            test_output_test3 = "test failed failure failed blabala3 hbs[dsfds.ets]fail error"
            err_output = ""
            expected_result = {("test1.ets", True), ("test2.ets", True), ("test3.ets", True), ("test4.ets", True),
                            ("test5_compile_only_neg.ets", True), ("test1.console.ets", True)}
            expected_stat = {"passed": 0, "ignored": 3, "failed": 3, "excluded": 0}

            config = self.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = list(runner.tests)

            for test in actual_tests:
                test.passed = False
                test.reports[ReportFormat.LOG] = "failure"
                if test.test_id == "test3.ets":
                    _ = BaseValidator.check_return_code(cast(TestStandardFlow, test), "step1", test_output_test3,
                                                                                err_output, 0)
                else:
                    _ = BaseValidator.check_return_code(cast(TestStandardFlow, test), "step1", test_output,
                                                        err_output, 0)

            runner.results = actual_tests
            failed = runner.summarize()
            actual_stat = {"passed": runner.passed, "ignored": runner.ignored,
                        "failed": runner.failed, "excluded": runner.excluded}

            self.assertEqual({(t.test_id, t.last_failure_check_passed) for t in actual_tests}, expected_result)
            self.assertEqual(failed, 3)
            test_utils.compare_dicts(self, expected_stat, actual_stat)

            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite1"],),
        (["runner.sh", "panda", "test_suite1", "--gn-build"],),
    ])
    def test_results_with_expected_failure_fail(self, argv: Callable) -> None:
        with patch('sys.argv', argv):
            test_output = "test unexpected failure"
            err_output = ""
            expected_result = {("test1.ets", False), ("test2.ets", True), ("test3.ets", False), ("test4.ets", True),
                            ("test5_compile_only_neg.ets", True), ("test1.console.ets", True)}
            expected_stat = {"passed": 0, "ignored": 1, "failed": 5, "excluded": 0}
            config = self.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = list(runner.tests)

            for test in actual_tests:
                test.passed = False
                test.reports[ReportFormat.LOG] = "failure"
                _ = BaseValidator.check_return_code(cast(TestStandardFlow, test), "step1", test_output,
                                                                                err_output, 0)
            runner.results = actual_tests
            failed = runner.summarize()
            actual_stat = {"passed": runner.passed, "ignored": runner.ignored,
                        "failed": runner.failed, "excluded": runner.excluded}

            self.assertEqual({(t.test_id, t.last_failure_check_passed) for t in actual_tests}, expected_result)
            self.assertEqual(failed, 5)
            test_utils.compare_dicts(self, expected_stat, actual_stat)

            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

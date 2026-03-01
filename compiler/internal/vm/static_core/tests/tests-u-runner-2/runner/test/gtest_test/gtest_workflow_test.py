#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

import os
import shutil
import sys
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.test import test_utils
from runner.test_base import GTest


class GTestFlowTest(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__)
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    test_list: ClassVar[Callable[[str], Path]] = lambda list_name: test_utils.get_list_path(__file__,
                                                                                            "ani_tests.txt")

    def config_test(self) -> Config:
        args = get_args()
        config = Config(args)
        return config

    def set_up_mock(self, mock_run: MagicMock, stdout: str = "", stderr: str = "", return_code: int = 0) -> None:
        mock_process = MagicMock()
        mock_process.returncode = return_code
        mock_process.stdout = stdout
        mock_process.stderr = stderr
        mock_run.return_value = mock_process

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.run")
    def test_gtest_runner_all_tests_in_class(self, mock_run: MagicMock) -> None:
        config = self.config_test()
        stdout = ("AnyCallTest.\n AnyCall_Valid\n AnyCall_NonFunctionObject\n AnyCall_WithClosure\n "
                  "AnyCall_NestedFunction\n AnyCall_RecursiveFunction\n AnyCall_InvalidFunc\n "
                  "AnyCall_InvalidArgsPointerWhenArgcNotZero\n AnyCall_NullResult\n "
                  "AnyCall_ZeroArgcWithNullArgv\n")

        self.set_up_mock(mock_run, stdout=stdout)

        runner = RunnerStandardFlow(config)
        tests = runner.tests
        self.assertEqual(len(tests), 9)

        for test in tests:
            test = cast(GTest, test)
            self.assertEqual(test.gtest_class, "AnyCallTest")
            self.assertIn(test.gtest_name, stdout)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file",
                        "ani_test_any_call/AnyCallTest.AnyCall_Valid"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_gtest_runner_one_tests(self) -> None:
        config = self.config_test()
        full_test_name = sys.argv[sys.argv.index("--test-file") + 1]
        test_file = full_test_name.split("/")[0]
        test_class, test_name = full_test_name.split("/")[1].split(".")
        path_to_test = test_utils.data_folder(__file__) / test_file

        runner = RunnerStandardFlow(config)
        tests = runner.tests
        test = next(iter(tests))
        test = cast(GTest, test)
        self.assertEqual(len(tests), 1)

        self.assertEqual(test.test_id, full_test_name)
        self.assertEqual(test.gtest_class, test_class)
        self.assertEqual(test.gtest_name, test_name)
        self.assertEqual(test.path, path_to_test)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_suite.TestSuite._set_explicit_list', test_list)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-list", "ani_test.txt"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_gtest_runner_test_list(self) -> None:
        config = self.config_test()
        runner = RunnerStandardFlow(config)
        test_from_list = "ani_test_any_call/AnyCallTest.AnyCall_RecursiveFunction"

        tests = runner.tests
        self.assertEqual(len(tests), 1)

        test = next(iter(tests))
        self.assertEqual(test.test_id, test_from_list)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

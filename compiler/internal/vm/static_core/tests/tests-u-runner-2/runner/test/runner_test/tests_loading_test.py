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
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import patch

from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.test import test_utils


class TestsLoadingTest(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__)
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)

    @staticmethod
    def get_config() -> Config:
        args = get_args()
        return Config(args)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite"],),
        (["runner.sh", "workflow", "test_suite", "--gn-build"],),
    ])
    def test_default_run(self, argv: Callable) -> None:
        """
        Expected: loaded all tests but test4_excluded which excluded one
        Tests from ignored test list (test1 and test5) are loaded
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                "test5_ignored_asan.ets",
                'test6_excluded_repeats.ets',
                'test7_excluded_asan_repeats.ets',
                'test8_co_neg.ets'
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite", "--skip-compile-only-neg"],),
        (["runner.sh", "workflow", "test_suite", "--skip-compile-only-neg", "--gn-build"],),
    ])
    def test_skip_compile_only_neg(self, argv: Callable) -> None:
        """
        Expected: loaded all tests but test4 which excluded one and test8_co_neg which is compile-only
        and negative
        Tests from ignored test list (test1 and test5) are loaded
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                "test5_ignored_asan.ets",
                'test6_excluded_repeats.ets',
                'test7_excluded_asan_repeats.ets'
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite", "--exclude-ignored-tests"],),
        (["runner.sh", "workflow", "test_suite", "--exclude-ignored-tests", "--gn-build"],),
    ])
    def test_exclude_all_ignored(self, argv: Callable) -> None:
        """
        Expected: loaded only test2 (normal test) and test3 (compile-only test)
        Tests from ignored test list (test1 and test5) are excluded like test3 (from excluded list)
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test2.ets",
                "test3_co.ets",
                "test7_excluded_asan_repeats.ets",
                "test8_co_neg.ets"
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite",
            "--exclude-ignored-test-lists=test_suite-ignored-ASAN.txt"],),
        (["runner.sh", "workflow", "test_suite",
            "--exclude-ignored-test-lists=test_suite-ignored-ASAN.txt", "--gn-build"],),
    ])
    def test_exclude_ignored_one_list(self, argv: Callable) -> None:
        """
        Expected: loaded only test2 (normal test) and test3 (compile-only test)
        Tests from ignored test list (test1 and test5) are excluded like test3 (from excluded list)
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                "test6_excluded_repeats.ets",
                "test7_excluded_asan_repeats.ets",
                "test8_co_neg.ets"
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite", "--exclude-ignored-test-lists=test_suite-*-ASAN.txt"],),
        (["runner.sh", "workflow", "test_suite", "--exclude-ignored-test-lists=test_suite-*-ASAN.txt",
            "--gn-build"],),
    ])
    def test_exclude_ignored_one_list_regex(self, argv: Callable) -> None:
        """
        Expected:
        Tests from ignored test list (test5_ignored_asan) is excluded like test4_excluded.ets (from excluded list)
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                "test6_excluded_repeats.ets",
                "test7_excluded_asan_repeats.ets",
                "test8_co_neg.ets"
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite",
            "--exclude-ignored-test-lists=test_suite-ignored-ASAN.txt",
            "--exclude-ignored-test-lists=test_suite-ignored-OL0.txt"],),
        (["runner.sh", "workflow", "test_suite",
            "--exclude-ignored-test-lists=test_suite-ignored-ASAN.txt",
            "--exclude-ignored-test-lists=test_suite-ignored-OL0.txt", "--gn-build"],),
    ])
    def test_exclude_ignored_2_lists(self, argv: Callable) -> None:
        """
        Expected: loaded all tests but following ones:
         - test1_ignored
         - test5_ignored_asan
         - test4_excluded
         - test6_excluded_repeats
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                'test7_excluded_asan_repeats.ets',
                'test8_co_neg.ets'
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite",
            "--exclude-ignored-tests", "--skip-compile-only-neg"],),
        (["runner.sh", "workflow", "test_suite",
            "--exclude-ignored-tests", "--skip-compile-only-neg", "--gn-build"],),
    ])
    def test_both_exclude_ignored_skip_compile_only_neg(self, argv: Callable) -> None:
        """
        Expected: following tests are loaded
        - test2 - ordinary test
        - test3_co - because it's just compile-only
        - test7_excluded_asan_repeats - the same
        Other tests are excluded:
        - test1_ignored and test5_ignored_asan because they are ignored and the option --exclude-ignored-tests is set
        - test4_excluded because it's excluded in the common exclude list
        - test8_co_neg because it's compile-only and negative
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test2.ets",
                "test3_co.ets",
                'test7_excluded_asan_repeats.ets'
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite", "--skip-test-lists"],),
        (["runner.sh", "workflow", "test_suite", "--skip-test-lists", "--gn-build"],),
    ])
    def test_skip_test_lists(self, argv: Callable) -> None:
        """
        Expected: loaded all tests, including test4 from excluded test list.
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                "test4_excluded.ets",
                "test5_ignored_asan.ets",
                'test6_excluded_repeats.ets',
                'test7_excluded_asan_repeats.ets',
                'test8_co_neg.ets'
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow-repeats", "test_suite"],),
        (["runner.sh", "workflow-repeats", "test_suite", "--gn-build"],),
    ])
    def test_repeats_40_lists(self, argv: Callable) -> None:
        """
        Expected: excluded test lists with REPEATS tag are loaded and tests test4_excluded, test6_excluded_repeats,
        test7_excluded_asan_repeats do not appear in the resulting list.
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test1_ignored.ets",
                "test2.ets",
                "test3_co.ets",
                "test5_ignored_asan.ets",
                "test8_co_neg.ets"
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow-repeats", "test_suite", "--exclude-ignored-tests"],),
        (["runner.sh", "workflow-repeats", "test_suite", "--exclude-ignored-tests", "--gn-build"],),
    ])
    def test_exclude_ignored_and_repeats_40_lists(self, argv: Callable) -> None:
        """
        Expected: only tests not mentions in any exclude or ignored test list are shown:
        - test2
        - test3_co
        - test8_co_neg
        All other tests should be excluded due to different excluded and ignored test lists.
        """
        with patch('sys.argv', argv):
            # preparation
            config = self.get_config()
            # test
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = [
                "test2.ets",
                "test3_co.ets",
                "test8_co_neg.ets"
            ]
            self.assertListEqual(actual_tests, expected_tests)
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

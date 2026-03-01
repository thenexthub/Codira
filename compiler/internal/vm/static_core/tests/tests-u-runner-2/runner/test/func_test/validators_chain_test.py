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
import os
import shutil
import unittest
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar, cast
from unittest.mock import MagicMock, patch

from runner.extensions.validators.base_validator import BaseValidator
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.test import test_utils


class ValidatorsChainTest(unittest.TestCase):
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__,
                                                                               "test_validators_data")
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', data_folder().as_posix())

    @staticmethod
    def get_runner_config() -> Config:
        return Config(get_args())

    @staticmethod
    def run_test() -> TestStandardFlow:
        config = ValidatorsChainTest.get_runner_config()
        runner = RunnerStandardFlow(config)
        actual_test = next(iter(runner.tests))
        test_result = actual_test.do_run()
        return cast(TestStandardFlow, test_result)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1_compile_only.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_compile_only_rt_fail(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test1_compile_only.ets
        validators: check_return_code - FAIL
        expected result - test fails because return code = 1
        '''
        test_utils.set_process_mock(mock_popen, return_code=1)

        test_result = self.run_test()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1_compile_only.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_compile_only_rt_stderr_fail(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test1_compile_only.ets
        validators: check_return_code - OK, check_stderr - FAIL
        expected result - test fails as stderr is not empty
        '''
        test_utils.set_process_mock(mock_popen, return_code=0, error_out="Error")

        test_result = self.run_test()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test2_compile_only.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_compile_only_rt_stderr_pass(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test2_compile_only.ets
        validators: check_return_code - OK,check_stderr - OK
        expected result: test passed, stderr is compared with expected stderr output from test's metadata
        '''
        test_utils.set_process_mock(mock_popen, return_code=0, error_out="Error")

        test_result = self.run_test()
        self.assertTrue(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test3_compile_only.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    @patch.object(BaseValidator, "check_return_code", wraps=BaseValidator.check_return_code)
    @patch.object(BaseValidator, "check_stderr", wraps=BaseValidator.check_return_code)
    @patch.object(BaseValidator, "check_output", wraps=BaseValidator.check_return_code)
    def test_compile_only_rt_stdout_pass(self, mock_check_stdout: MagicMock,
                                         mock_check_stderr: MagicMock,
                                         mock_check_rt: MagicMock,
                                         mock_popen: MagicMock) -> None:
        '''
        test-file: test3_compile_only.ets
        validators: check_return_code - OK, check_stderr - OK, check_stdout - OK
        expected result: test passes
        '''
        test_utils.set_process_mock(mock_popen, return_code=0, output="some text")

        test_result = self.run_test()
        mock_check_rt.assert_called_once()
        mock_check_stderr.assert_called_once()
        mock_check_stdout.assert_called_once()
        self.assertTrue(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_runtime_pass(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test4_runtime.ets, stdout for compile step
        validators:
            - compile/verifier steps: check_return_code - OK, check_stderr - OK
            - runtime(runtime/aot) steps: check_return_code - OK, check_stderr - OK
        expected result: test passes, stdout is not compared as there is no data to compare with
        '''
        test_utils.set_process_mock(mock_popen, return_code=0, output="some text")

        test_result = self.run_test()
        self.assertTrue(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_runtime_rt_fail(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test4_runtime.ets
        validators:
            - compiler/ verifier: check_return_code - OK, check_stderr - OK
            - aot: check_return_code - FAIL
        expected result - test fails because AOT step return_code = 1
        '''
        mock1 = MagicMock(name="popen1")
        mock2 = MagicMock(name="popen2")

        for m, out, rt in ((mock1, ("", ""), 0), (mock2, ("", ""), 1)):
            m.__enter__.return_value = m
            m.communicate.return_value = out
            m.returncode = rt

        mock_popen.side_effect = [mock1, mock1, mock2, mock1]

        test_result = self.run_test()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test5_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_runtime_stdout_fail(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test5_runtime.ets, stdout in aot step
        validators:
            - compiler/ verifier: check_return_code - OK, check_stderr - OK
            - aot: check_return_code - OK, check_stderr - OK, check_stdout - fail
        expected result: tests fails as a result of comparison of the stdout in aot step
        '''
        mock1 = MagicMock(name="popen1")
        mock2 = MagicMock(name="popen2")

        for m, out in ((mock1, ("", "")), (mock2, ("test", ""))):
            m.__enter__.return_value = m
            m.communicate.return_value = out
            m.returncode = 0

        mock_popen.side_effect = [mock1, mock1, mock2, mock1]

        test_result = self.run_test()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test5_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_runtime_stdout_last_step_fail(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test5_runtime.ets, stdout in runtime step (last step)
        validators:
            - compiler/ verifier: check_return_code - OK, check_stderr - OK
            - aot: check_return_code - OK, check_stderr - OK
            - runtime: check_return_code - OK, check_stderr - OK, check_stdout - fail
        expected result: tests fails as a result of comparison of the stdout in the last step
        '''
        mock1 = MagicMock(name="popen1")
        mock2 = MagicMock(name="popen2")

        for m, out in ((mock1, ("", "")), (mock2, ("test", ""))):
            m.__enter__.return_value = m
            m.communicate.return_value = out
            m.returncode = 0

        mock_popen.side_effect = [mock1, mock1, mock1, mock2]

        test_result = self.run_test()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test5_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    @patch.object(BaseValidator, "check_return_code", wraps=BaseValidator.check_return_code)
    @patch.object(BaseValidator, "check_stderr", wraps=BaseValidator.check_return_code)
    @patch.object(BaseValidator, "check_output", wraps=BaseValidator.check_return_code)
    def test_runtime_fail_on_compile(self, mock_check_stdout: MagicMock,
                                     mock_check_stderr: MagicMock,
                                     mock_check_rc: MagicMock,
                                     mock_popen: MagicMock) -> None:
        '''
        test-file: test5_runtime.ets, stdout in runtime step (last step), expected to fail on comparison
        actual fail on compile step
        validators:
            - compiler: check_return_code - fail
        expected result: tests fails as compile step fails
        '''
        mock1 = MagicMock(name="popen1")
        mock2 = MagicMock(name="popen2")

        for m, out, rt in ((mock1, ("", ""), 1), (mock2, ("test", ""), 0)):
            m.__enter__.return_value = m
            m.communicate.return_value = out
            m.returncode = rt

        mock_popen.side_effect = [mock1, mock1, mock1, mock2]

        test_result = self.run_test()
        mock_check_rc.assert_called_once()
        mock_check_stdout.assert_not_called()
        mock_check_stderr.assert_not_called()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test5_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_runtime_stdout_pass(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test5_runtime.ets, stdout in aot step
        validators:
            - compiler/ verifier: check_return_code - OK, check_stderr - OK
            - aot: check_return_code - OK, check_stderr - OK, check_stdout - OK
            - runtime: check_return_code - OK, check_stderr - OK
        expected result: tests passes
        '''
        mock1 = MagicMock(name="popen1")
        mock2 = MagicMock(name="popen2")

        for m, out in ((mock1, ("", "")), (mock2, ("some text", ""))):
            m.__enter__.return_value = m
            m.communicate.return_value = out
            m.returncode = 0

        mock_popen.side_effect = [mock1, mock1, mock2, mock1]

        test_result = self.run_test()
        self.assertTrue(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test5_runtime.ets"])
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_runtime_no_stdout_fail(self, mock_popen: MagicMock) -> None:
        '''
        test-file: test5_runtime.ets, no stdout, but expected comparison with stdout
        validators:
            - compiler/ verifier: check_return_code - OK, check_stderr - OK
            - aot: check_return_code - OK, check_stderr - OK, check_stdout - FAIL( but ignore as there is one more step)
            - runtime: check_return_code - OK, check_stderr - OK, check_stdout - FAIL
        expected result: tests fails
        '''
        mock1 = MagicMock(name="popen1")

        mock1.__enter__.return_value = mock1
        mock1.communicate.return_value = ("", "")
        mock1.returncode = 0

        mock_popen.side_effect = [mock1, mock1, mock1, mock1]

        test_result = self.run_test()
        self.assertFalse(test_result.passed)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

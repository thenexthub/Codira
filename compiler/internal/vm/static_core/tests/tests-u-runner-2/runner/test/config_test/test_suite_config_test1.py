#!/usr/bin/env python3
# -- coding: utf-8 --
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
import unittest
from pathlib import Path
from typing import ClassVar
from unittest.mock import patch

from runner.options import cli_options_utils as cli_utils
from runner.options.cli_options import CliOptionsParser, CliParserBuilder, ConfigsLoader
from runner.test.config_test.data import data_1, data_2
from runner.test.test_utils import compare_dicts


class TestSuiteConfigTest1(unittest.TestCase):
    workflow_name = "config-1"
    test_suite_name = "test_suite1"
    current_path = Path(__file__).parent
    data_folder = current_path / "data"
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': Path.cwd().as_posix()
    }

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest1.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest1.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_min_args(self) -> None:
        configs = ConfigsLoader(self.workflow_name, self.test_suite_name)

        parser_builder = CliParserBuilder(configs)
        test_suite_parser, key_lists_ts = parser_builder.create_parser_for_test_suite()
        workflow_parser, key_lists_wf = parser_builder.create_parser_for_workflow()

        cli = CliOptionsParser(configs, parser_builder.create_parser_for_runner(),
                               test_suite_parser,
                               parser_builder.create_parser_for_default_test_suite(),
                               workflow_parser, *[])
        cli.parse_args()

        actual = cli.full_options
        actual = cli_utils.restore_duplicated_options(configs, actual)
        actual = cli_utils.add_config_info(configs, actual)
        actual = cli_utils.restore_default_list(actual, key_lists_ts | key_lists_wf)
        expected = data_1.args
        compare_dicts(self, actual, expected)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest1.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest1.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_args_urunner(self) -> None:
        args = [
            "--show-progress",
            "--verbose", "short",
            "--processes", "12",
            "--detailed-report",
            "--detailed-report-file", "my-report",
            "--report-dir", "my-report-dir",
            "--verbose-filter", "ignored",
            "--enable-time-report",
            "--qemu", "arm64",
            "--report-dir", "my-report-dir",
            "--use-llvm-cov",
            "--use-lcov",
            "--llvm-cov-exclude", "/tmp",
            "--llvm-cov-exclude", "*.h",
            "--lcov-exclude", "/tmp",
            "--lcov-exclude", "*.h",
            "--profdata-files-dir", ".",
            "--coverage-html-report-dir", ".",
            "--coverage-per-binary",
            "--clean-gcda-before-run",
            "--time-edges", "1,10,100,500",
            "--gn-build",
            "--gcov-tool", "/usr/bin/ls"]

        configs = ConfigsLoader(self.workflow_name, self.test_suite_name)

        parser_builder = CliParserBuilder(configs)
        test_suite_parser, key_lists_ts = parser_builder.create_parser_for_test_suite()
        workflow_parser, key_lists_wf = parser_builder.create_parser_for_workflow()

        cli = CliOptionsParser(configs, parser_builder.create_parser_for_runner(),
                               test_suite_parser,
                               parser_builder.create_parser_for_default_test_suite(),
                               workflow_parser, *args)
        cli.parse_args()

        actual = cli.full_options
        actual = cli_utils.restore_duplicated_options(configs, actual)
        actual = cli_utils.add_config_info(configs, actual)
        actual = cli_utils.restore_default_list(actual, key_lists_ts | key_lists_wf)
        expected = data_2.args
        compare_dicts(self, actual, expected)

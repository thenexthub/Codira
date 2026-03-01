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
from io import StringIO
from pathlib import Path
from typing import ClassVar, cast
from unittest.mock import patch

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.base_enum import IncorrectEnumValue
from runner.enum_types.verbose_format import VerboseKind
from runner.options import cli_options_utils as cli_utils
from runner.options.cli_options import CliOptions, CliOptionsParser, CliParserBuilder, ConfigsLoader


class TestSuiteConfigTest2(unittest.TestCase):
    workflow_name = "config-1"
    test_suite_name = "test_suite2"
    current_path = Path(__file__).parent
    data_folder = current_path / "data"
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': Path.cwd().as_posix()
    }

    def test_empty_args(self) -> None:
        args: list[str] = []
        with self.assertRaises(InvalidConfiguration):
            CliOptions(args)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_only_workflow(self) -> None:
        args = [self.workflow_name]
        with self.assertRaises(InvalidConfiguration):
            CliOptions(args)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch("sys.stderr", new_callable=StringIO)
    def test_option_without_value(self, handle: StringIO) -> None:
        args = [self.workflow_name, self.test_suite_name, "--verbose"]
        with self.assertRaises(IncorrectEnumValue):
            try:
                CliOptions(args)
            except IncorrectEnumValue as exc:
                std_err = str(handle.getvalue())
                pos = std_err.find("error: argument --verbose/-v: expected one argument")
                self.assertGreaterEqual(pos, 0, "Error message about `--verbose` is not found")
                raise IncorrectEnumValue(str(exc.args)) from exc

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_option_with_incorrect_value(self) -> None:
        args = [self.workflow_name, self.test_suite_name, "--verbose", "abc"]
        with self.assertRaises(IncorrectEnumValue):
            CliOptions(args)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_option_with_repeated_value(self) -> None:
        args = [
            "--verbose", "all", "--verbose", "short"
        ]
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
        self.assertEqual(VerboseKind.SHORT, actual.get("runner.verbose"))

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_option_with_test_suite_option1(self) -> None:
        args = [
            "--with-js"
        ]
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
        self.assertTrue(actual.get('test_suite2.parameters.with-js'))

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_option_with_test_suite_option3(self) -> None:
        args = [
            "--validator", "a.b.c"
        ]
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
        self.assertEqual("a.b.c", actual.get('test_suite2.parameters.validator'))

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_option_with_test_suite_option_several_times1(self) -> None:
        args = [
            "--validator", "a.b.c", "--validator", "d.e.f"
        ]

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

        self.assertEqual("d.e.f", actual.get('test_suite2.parameters.validator'))

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest2.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_option_with_test_suite_option_several_times2(self) -> None:
        args = [
            "--es2panda-full-args=--thread=0", "--es2panda-full-args=--parse-only", "--es2panda-full-args=--dump-ast",
            "--es2panda-full-args=--extension=${parameters.extension}"
        ]
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

        expected = ['--thread=0', '--parse-only', '--dump-ast', '--extension=${parameters.extension}']
        self.assertEqual(sorted(expected),
                         sorted(cast(list, actual.get('test_suite2.parameters.es2panda-full-args'))))

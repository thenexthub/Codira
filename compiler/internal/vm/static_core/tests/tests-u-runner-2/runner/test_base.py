#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

from __future__ import annotations

import argparse
import re
from argparse import ArgumentParser
from datetime import datetime
from pathlib import Path, PurePosixPath
from typing import Any, Literal, cast

import pytz

from runner import utils
from runner.common_exceptions import ParameterNotFound
from runner.enum_types.base_enum import BaseEnum
from runner.enum_types.params import TestEnv, TestReport
from runner.enum_types.verbose_format import VerboseFilter, VerboseKind
from runner.logger import Log
from runner.options.local_env import LocalEnv
from runner.options.macros import Macros
from runner.options.options import IOptions
from runner.options.options_general import GeneralOptions
from runner.reports.report import ReportGenerator
from runner.reports.report_format import ReportFormat

_LOGGER = Log.get_logger(__file__)


class TestStatus(BaseEnum):
    PASSED = "PASSED"
    PASSED_IGNORED = "PASSED IGNORED"
    FAILURE_IGNORED = "FAILURE IGNORED"
    EXCLUDED = "EXCLUDED"
    NEW_FAILURE = "NEW FAILURE"


class Test:
    """
    Test knows how to run test and defines whether the test is failed or passed
    """

    def __init__(self, test_env: TestEnv, test_path: Path, params: IOptions, test_id: str):
        self.test_env = test_env
        # full path to the test file
        self.path = test_path
        self.test_extra_params: IOptions = params
        self.test_id = test_id
        self.update_expected = self.test_env.config.test_suite.test_lists.update_expected
        # Expected output. Used in the Parser and CTS test suites
        self.expected: str = ""
        self.expected_err: str = ""
        # Contains fields output, error, and return_code of the last executed step
        self.report: TestReport | None = None
        # Test result: True if all steps passed, False is any step fails
        self.passed: bool | None = None
        # If the test is mentioned in any ignored_list
        self.ignored = False
        # failure description for a test from ignore list (if any)
        self.last_failure: str = ""
        # The result of last failure check for the test from ignore list
        self.last_failure_check_passed: bool = True
        # Test can detect itself as excluded additionally to excluded_tests
        # In such case the test will be counted as `excluded by other reasons`
        self.excluded = False
        # Collect all executable commands
        self.reproduce = ""
        # Time to execute in seconds
        self.time: float | None = None
        # Reports if generated. Key is ReportFormat.XXX. Value is a path to the generated report
        self.reports: dict[ReportFormat, str] = {}
        self.repeat = 1
        self.path_to_expected = Path(f"{self.path}.expected")
        self.path_to_expected_err = Path(f"{self.path}.expected.err")

    @property
    def has_expected(self) -> bool:
        """ True if test.expected file exists """
        return self.path_to_expected.exists()

    @property
    def has_expected_err(self) -> bool:
        """ True if test.expected.err file exists """
        return self.path_to_expected_err.exists()

    @staticmethod
    def prepare_config_parser(prs: ArgumentParser, params: dict) -> ArgumentParser:
        for key, val in params.items():
            if isinstance(val, bool):
                prs.add_argument(f'--{key}',
                                               dest=f"{key}", action='store_true')
            elif isinstance(val, list):
                prs.add_argument(f'--{key}', dest=f"{key}", action='append')
            else:
                prs.add_argument(f'--{key}', dest=f"{key}")

        return prs

    def run(self, repeat: int) -> Test | GTest:
        start = datetime.now(pytz.UTC)
        _LOGGER.all(f"\033[1mStarted:\033[0m \033[1;33m{self.test_id}\033[0m. Launch #{repeat}")
        self.repeat = repeat
        self.reproduce = ''

        result = self.do_run()

        finish = datetime.now(pytz.UTC)
        self.time = (finish - start).total_seconds()

        if result.passed is None:
            return result

        if self.is_output_status():
            Log.default(
                _LOGGER,
                f"\033[1mFinished:\033[0m \033[1;33m{self.test_id}\033[0m. Launch #{repeat}"
                f"- {round(self.time, 2)} sec - {self.status_as_cstring()}")
        self.reproduce += f"\nTo reproduce with URunner run:\n{self.get_command_line()}"
        self.reproduce += f"\nProvide following environment variables: {LocalEnv.get().list()}"
        if self.is_output_log():
            _LOGGER.default(f"{self.test_id}: steps: {self.reproduce}")
            if not self.report:
                Log.default(
                    _LOGGER,
                    f"{self.test_id}: no information about test running neither output nor error")

        report_generator = ReportGenerator(self.test_id, self.test_env, repeat)
        self.reports = report_generator.generate_reports(result)

        return result

    def do_run(self) -> Test | GTest:
        return self

    def status(self) -> TestStatus:
        if self.excluded:
            return TestStatus.EXCLUDED

        if self.passed:
            return TestStatus.PASSED_IGNORED if self.ignored else TestStatus.PASSED

        if self.ignored:
            return TestStatus.FAILURE_IGNORED if self.last_failure_check_passed else TestStatus.NEW_FAILURE

        return TestStatus.NEW_FAILURE

    def status_as_cstring(self) -> str:
        if self.excluded:
            return TestStatus.EXCLUDED.value
        if self.passed:
            return f"\033[3;92m{TestStatus.PASSED_IGNORED.value}\033[0m" \
                if self.ignored else f"\033[1;32m{TestStatus.PASSED.value}\033[0m"
        return f"\033[3;91m{TestStatus.FAILURE_IGNORED.value}\033[0m" \
            if self.ignored else f"\033[1;31m{TestStatus.NEW_FAILURE.value}\033[0m"

    def get_command_line(self) -> str:
        reproduce_message = [
            f'(min parameters set): {self.get_short_cli()}',
            f'(full parameters set): {self.get_full_cli()}'
        ]

        return '\n'.join(reproduce_message)

    def get_short_cli(self) -> str:
        reproduce_message = [f'{Path(__file__).parent.parent}/runner.sh',
                             f'{self.test_env.config.workflow.name}', f'{self.test_env.config.test_suite.suite_name}',
                             f'{" ".join(self.test_env.config.general.cli_options)}']
        if '--test-file' not in ' '.join(reproduce_message):
            reproduce_message.append(f" --test-file {self.test_id}")

        return ' '.join(reproduce_message)

    def get_full_cli(self) -> str:
        reproduce_message = [
            f'{Path(__file__).parent.parent}/runner.sh',
            f'{self.test_env.config.workflow.name}',
            f'{self.test_env.config.test_suite.suite_name}']

        workflow_params = self.get_entity_params("workflow")
        test_suite_main_params = self.get_entity_params("test-suite")
        test_suite_collections_params = self.get_collection_params()
        runner_params = self.get_actual_runner_params()

        for key, val in workflow_params.items():
            if isinstance(val, list):
                for i, val_elem in enumerate(val):
                    val[i] = Macros.correct_macro(val_elem, self.test_env.config.workflow)
                workflow_params[key] = val
            else:
                workflow_params[key] = Macros.correct_macro(val, self.test_env.config.workflow)

        # workflow params has higher priority than test suite params
        #  the workflow values will override the test-suite values in case they both present in config files
        all_params = {**test_suite_main_params, **test_suite_collections_params, **workflow_params, **runner_params}
        all_params = {f"--{key}": val.value if isinstance(val, BaseEnum) else val for key, val in all_params.items()}
        all_params_list = utils.convert_dict_params_into_list(utils.delete_filtering_options_nested_keys(all_params))
        reproduce_message.extend(all_params_list)

        if '--test-file' not in ' '.join(reproduce_message):
            reproduce_message.append(f"--test-file {self.test_id}")

        return ' '.join(reproduce_message)

    def get_entity_params(self,                         # type: ignore[explicit-any]
                          entity: Literal["workflow", "test-suite"]) -> dict[str, Any]:
        match entity:
            case "workflow":
                data = f"{utils.convert_minus(self.test_env.config.workflow.name)}.data"
            case "test-suite":
                data = f"{utils.convert_minus(self.test_env.config.test_suite.suite_name)}.data"
            case _:
                raise ParameterNotFound

        return cast(dict, getattr(self.test_env.config.general, data)["parameters"])

    def get_actual_runner_params(self) -> dict:
        cli_options = self.test_env.config.general.cli_options
        cli_options_splited = []
        for option in cli_options:
            if "=" in option:
                cli_options_splited.extend(option.split("=", 1))
            else:
                cli_options_splited.append(option)

        runner_parser = argparse.ArgumentParser(description="Runner args parser")
        GeneralOptions.add_cli_args(runner_parser)
        runner_args, other_args = runner_parser.parse_known_args(cli_options)

        test_suite_parser = argparse.ArgumentParser(description="Test Suite args parser")
        test_suite_parser = Test.prepare_config_parser(test_suite_parser, self.test_env.config.test_suite.parameters)
        test_suite_args, other_args = test_suite_parser.parse_known_args(other_args)

        workflow_parser = argparse.ArgumentParser(description="Workflow args parser")
        workflow_parser = Test.prepare_config_parser(workflow_parser, self.test_env.config.workflow.parameters)

        workflow_args, other_args = workflow_parser.parse_known_args(other_args)

        return {**{key: val for key, val in vars(runner_args).items() if f"--{key}" in cli_options_splited},
                **{key: val for key, val in vars(test_suite_args).items() if f"--{key}" in cli_options_splited},
                **{key: val for key, val in vars(workflow_args).items() if f"--{key}" in cli_options_splited}}

    def get_collection_params(self) -> dict:
        """ there might be additional params for the test-suite in the collections section"""
        collection_params: dict[str, list] = {}
        collections = self.test_env.config.test_suite.collections
        for collection in collections:
            if not collection.parameters:
                continue
            for key, val in collection.parameters.items():
                if isinstance(val, list):
                    val = " ".join(val)
                if key in collection_params:
                    collection_params[key] += [*val.split()]
                else:
                    val = str(val)
                    collection_params[key] = [*val.split()]

        return collection_params

    def is_output_log(self) -> bool:
        """
        Returns True if logs should be logged
        verbose in [ALL, SHORT] and filter == ALL -> True
        verbose in [ALL, SHORT] and filter == KNOWN and status = New or Known or Passed Ignored -> True
        verbose in [ALL, SHORT] and filter == NEW and status = New -> True

        verbose == NONE and status == New -> True
        """
        verbose_filter = self.test_env.config.general.verbose_filter
        verbose = self.test_env.config.general.verbose
        status = self.status()
        if verbose in [VerboseKind.ALL, VerboseKind.SHORT]:
            is_all = verbose_filter == VerboseFilter.ALL
            is_ignored = verbose_filter == VerboseFilter.IGNORED and \
                         status in [TestStatus.NEW_FAILURE, TestStatus.FAILURE_IGNORED, TestStatus.PASSED_IGNORED]
            is_new = verbose_filter == VerboseFilter.NEW_FAILURES and \
                     status == TestStatus.NEW_FAILURE
            return is_all or is_ignored or is_new

        return verbose == VerboseKind.SILENT and status == TestStatus.NEW_FAILURE

    def is_output_status(self) -> bool:
        """
        Returns True if status should be logged
        verbose in [ALL, SHORT] -> True

        verbose == NONE and filter == ALL -> True
        verbose == NONE and filter == KNOWN and status = New or Known or Passed Ignored -> True
        verbose == NONE and filter == NEW and status = New -> True
        """
        verbose = self.test_env.config.general.verbose
        verbose_filter = self.test_env.config.general.verbose_filter
        status = self.status()
        if verbose == VerboseKind.SILENT:
            is_all = verbose_filter == VerboseFilter.ALL
            is_ignored = verbose_filter == VerboseFilter.IGNORED and \
                         status in [TestStatus.NEW_FAILURE, TestStatus.FAILURE_IGNORED, TestStatus.PASSED_IGNORED]
            is_new = verbose_filter == VerboseFilter.NEW_FAILURES and \
                     status == TestStatus.NEW_FAILURE
            return is_all or is_ignored or is_new

        return True

    def get_test_names(self) -> tuple[str, str]:

        # if --test-file is set to test, e.g. ani_test_any_call/AnyTestClass.ani_test_valid
        test_id_re = re.compile(r'^[A-Z][A-Za-z0-9_]*\.[A-Za-z][A-Za-z0-9_]+$')
        if test_id_re.match(PurePosixPath(self.test_id).name):
            test_class_name, test_name = PurePosixPath(self.test_id).name.split(".")
            return test_class_name, test_name

        return "", ""


class GTest(Test):
    gtest_name: str
    gtest_class: str
    precompiled_tests: bool

    def __init__(self, test_env: TestEnv, test_path: Path, params: IOptions, test_id: str, precompiled_tests: bool):
        Test.__init__(self, test_env, test_path, params, test_id)
        self.path = self.path.parent
        gtest_class, gtest_name = self.get_test_names()
        self.gtest_class = gtest_class
        self.gtest_name = gtest_name
        self.build_dir = Path(cast(str, self.test_env.config.workflow.get_parameter("build")))
        self.precompiled_tests = precompiled_tests

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

import subprocess
import os
import logging
from pathlib import Path
from typing import Any, List, Set

import yaml

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.ets.ets_test_suite import (
    EtsTestSuite,
    CtsEtsTestSuite,
    FuncEtsTestSuite,
    RuntimeEtsTestSuite
)
from runner.plugins.ets.ets_test_dir import EtsTestDir
from runner.plugins.declgenets2ts.test_declgenets2ts import DeclgenEts2tsStage, TestDeclgenETS2TS
from runner.plugins.declgenets2ts.declgenets2ts_suites import DeclgenEtsSuites
from runner.runner_base import get_test_id
from runner.runner_file_based import RunnerFileBased
from runner.enum_types.test_directory import TestDirectory
from runner.plugins.work_dir import WorkDir
from runner.plugins.ets.preparation_step import JitStep, \
    CopyStep

_LOGGER = logging.getLogger(
    "runner.plugins.declgenets2ts.runner_declgenets2ts")


class DeclgenSdkEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, DeclgenEtsSuites.DECLGENSDK.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.declgen_sdk,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
        ))


class RunnerDeclgenETS2TS(RunnerFileBased):
    def __init__(self, config: Config):
        config.verifier.enable = False
        self.__declgen_ets_name = self.get_declgen_ets_name(config.test_suites)

        super().__init__(config, self.__declgen_ets_name)

        self.declgen_ets2ts_executor = os.path.join(
            self.build_dir, "bin", "declgen_ets2ts")

        test_suite_class = self.get_declgen_ets_class(self.__declgen_ets_name)
        if self.__declgen_ets_name == DeclgenEtsSuites.RUNTIME.value:
            self.default_list_root = self._get_frontend_test_lists()
        self.declgen_list_root = os.path.join(self.default_list_root, "declgenets2ts")
        test_suite = test_suite_class(
            self.config, self.work_dir, self.declgen_list_root)
        test_suite.process(
            self.config.general.generate_only or self.config.ets.force_generate)
        self.test_root, self.list_root = test_suite.test_root, test_suite.list_root

        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.collect_ignored_excluded_tests()

        self.add_directories([TestDirectory(self.test_root, "ets", [])])

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp/declgenets2ts") / self.__declgen_ets_name

    def collect_ignored_excluded_tests(self) -> None:
        self.collect_excluded_test_lists(test_name=self.__declgen_ets_name)
        self.collect_ignored_test_lists(test_name=self.__declgen_ets_name)

        if self.__declgen_ets_name == "declgen-ets2ts-cts":
            self.ignored_cts_ets_prefix = "ets-cts"
        elif self.__declgen_ets_name == "declgen-ets2ts-func-tests":
            self.ignored_cts_ets_prefix = "ets-func-tests"
        elif self.__declgen_ets_name == "declgen-ets2ts-runtime":
            self.ignored_cts_ets_prefix = "ets-runtime"
        else:
            return

        self.list_root = os.path.join(self.default_list_root, self.ignored_cts_ets_prefix)
        self.collect_excluded_test_lists(test_name=self.ignored_cts_ets_prefix)
        self.excluded_lists.extend(self.collect_test_lists("ignored", None, self.ignored_cts_ets_prefix))

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestDeclgenETS2TS:
        test = TestDeclgenETS2TS(self.test_env, test_file, flags, get_test_id(
            test_file, self.test_root), self.build_dir, self.__declgen_ets_name)
        test.ignored = is_ignored
        return test

    def run_command(self, command: list, cwd: str, description: str, timeout: int = 60 * 10) -> None:
        Log.summary(_LOGGER, f"Running command: {' '.join(command)}")
        Log.summary(_LOGGER, description)
        with subprocess.Popen(
                command,
                cwd=cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.PIPE) as process:
            try:
                stdout, stderr = process.communicate(timeout=timeout)
                stdout_utf8 = stdout.decode("utf-8", errors="ignore")
                stderr_utf8 = stderr.decode("utf-8", errors="ignore")
                if process.returncode != 0:
                    Log.exception_and_raise(
                        _LOGGER, f"invalid return code {process.returncode}\n {stdout_utf8}\n {stderr_utf8}\n")
            except subprocess.TimeoutExpired as ex:
                Log.exception_and_raise(
                    _LOGGER, f"Timeout: {' '.join(command)} failed with {ex}")
            except Exception as ex:  # pylint: disable=broad-except
                Log.exception_and_raise(
                    _LOGGER, f"Exception{' '.join(command)} failed with {ex}")

    def read_package_info(self, package_json_path: str) -> str:
        try:
            with open(package_json_path, "r", encoding="utf-8") as file:
                package_data = yaml.safe_load(file)
            name = package_data.get("name")
            version = package_data.get("version")

            if not name or not version:
                Log.exception_and_raise(
                    _LOGGER, "The 'name' or 'version' field is missing in the YAML file.")

            package_name = f"{name}-{version}.tgz"
            return package_name
        except Exception as ex:  # pylint: disable=broad-except
            Log.exception_and_raise(_LOGGER, f"Error reading YAML file: {ex}")

    def get_declgen_ets_name(self, test_suites: Set[str]) -> str:
        name = ""
        if "declgen_ets2ts_cts" in test_suites:
            name = DeclgenEtsSuites.CTS.value
        elif "declgen_ets2ts_func_tests" in test_suites:
            name = DeclgenEtsSuites.FUNC.value
        elif "declgen_ets2ts_runtime" in test_suites:
            name = DeclgenEtsSuites.RUNTIME.value
        elif "declgen_ets2ts_sdk" in test_suites:
            name = DeclgenEtsSuites.DECLGENSDK.value
        else:
            Log.exception_and_raise(
                _LOGGER, f"Unsupported test suite: {self.config.test_suites}")
        return name

    def get_declgen_ets_class(self, ets_suite_name: str) -> Any:
        name_to_class = {
            DeclgenEtsSuites.CTS.value: CtsEtsTestSuite,
            DeclgenEtsSuites.FUNC.value: FuncEtsTestSuite,
            DeclgenEtsSuites.RUNTIME.value: RuntimeEtsTestSuite,
            DeclgenEtsSuites.DECLGENSDK.value: DeclgenSdkEtsTestSuite,
        }
        return name_to_class.get(ets_suite_name)

    def run(self) -> None:
        super().run()
        TestDeclgenETS2TS.results.extend(self.results)
        TestDeclgenETS2TS.state = DeclgenEts2tsStage.TSC
        self.tests.clear()
        for result in self.results:
            if result.passed:
                self.tests.add(result)
        super().run()
        path_to_result = {result.path: result for result in self.results}
        for i, result in enumerate(TestDeclgenETS2TS.results):
            if result.path in path_to_result:
                TestDeclgenETS2TS.results[i] = path_to_result[result.path]
        self.results = TestDeclgenETS2TS.results

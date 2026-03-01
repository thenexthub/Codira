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

import os
import logging
from pathlib import Path
from typing import Any, List, Set

import yaml

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.declgents2ets.declgents2ets_test_suite import DeclgenCtsTsTestSuite
from runner.plugins.declgents2ets.declgents2ets_suites import DeclgenTsSuites, DeclgenTsSuitesDir
from runner.plugins.declgents2ets.test_declgents2ets import TestDeclgenTS2ETS
from runner.runner_base import get_test_id
from runner.runner_file_based import RunnerFileBased
from runner.enum_types.test_directory import TestDirectory


_LOGGER = logging.getLogger(
    "runner.plugins.declgents2ets.runner_declgents2ets")


class RunnerDeclgenTS2ETS(RunnerFileBased):
    def __init__(self, config: Config):
        config.verifier.enable = False
        self.__ts_suite_name = self.get_ts_suite_name(config.test_suites)
        self.__ts_suite_dir = self.get_ts_suite_dir(config.test_suites)

        super().__init__(config, self.__ts_suite_name)

        self.declgen_ts2ets_executor = os.path.join(
            self.build_dir, "plugins", "ets", "tools", "declgen_ts2sts", "dist", "declgen.js")
        self.es2panda_executor = os.path.join(
            self.build_dir, "bin", "es2panda")

        test_suite_class = self.get_declgen_ts_class(self.__ts_suite_name)
        test_suite = test_suite_class(
            self.config, self.work_dir, self.default_list_root)
        test_suite.process(
            self.config.general.generate_only or self.config.ets.force_generate)
        self.test_root, self.list_root = test_suite.test_root, test_suite.list_root

        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.collect_excluded_test_lists(test_name=self.__ts_suite_name)
        self.collect_ignored_test_lists(test_name=self.__ts_suite_name)

        self.add_directories([TestDirectory(self.test_root, "ts", [])])

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / self.__ts_suite_dir / self.__ts_suite_name

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestDeclgenTS2ETS:
        test = TestDeclgenTS2ETS(self.test_env, test_file, flags, get_test_id(
            test_file, self.test_root), self.build_dir)
        test.ignored = is_ignored
        return test

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

    def get_ts_suite_name(self, test_suites: Set[str]) -> str:
        name = ""
        if "declgen_ts2ets_cts" in test_suites:
            name = DeclgenTsSuites.CTS.value
        else:
            Log.exception_and_raise(
                _LOGGER, f"Unsupported test suite: {self.config.test_suites}")
        return name

    def get_ts_suite_dir(self, test_suites: Set[str]) -> str:
        suite_dir = ""
        if "declgen_ts2ets_cts" in test_suites:
            suite_dir = DeclgenTsSuitesDir.CTS.value
        else:
            Log.exception_and_raise(
                _LOGGER, f"Unsupported test suite: {self.config.test_suites}")
        return suite_dir

    def get_declgen_ts_class(self, declgen_ts_suite_name: str) -> Any:
        name_to_class = {
            DeclgenTsSuites.CTS.value: DeclgenCtsTsTestSuite
        }
        return name_to_class.get(declgen_ts_suite_name)

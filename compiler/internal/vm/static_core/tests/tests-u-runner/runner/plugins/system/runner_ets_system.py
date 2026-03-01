#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

import logging
from os import path
from enum import Enum
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.system.test_ets_system import TestETSSystem
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger("runner.plugins.system.runner_ets_system")


class SystemArkTSFlags(Enum):
    ALL_WARNINGS = ["--ets-warnings"]
    BOOST_EQUALITY_STATEMENTS = ["--ets-warnings:enable=ets-boost-equality-statement"]
    IMPLICIT_BOXING_UNBOXING = ["--ets-warnings:enable=ets-implicit-boxing-unboxing"]
    PROHIBIT_TOP_LEVEL_STATEMENTS = ["--ets-warnings:enable=ets-prohibit-top-level-statements"]
    REMOVE_ASYNC = ["--ets-warnings:enable=ets-remove-async"]
    REMOVE_LAMBDA = ["--ets-warnings:enable=ets-remove-lambda"]
    SUGGEST_FINAL = ["--ets-warnings:enable=ets-suggest-final"]
    SUPPRESSION_TESTS = ["--ets-warnings"]
    WERROR_TESTS = ["--ets-warnings:werror"]


class RunnerETSSystem(RunnerJS):
    def __init__(self, config: Config) -> None:
        super().__init__(config, "system")

        symlink_es2panda_test = (Path(config.general.static_core_root) /
                                 "plugins" / "ets" / "tests" / "ets_warnings_tests")
        if symlink_es2panda_test.exists():
            es2panda_test = symlink_es2panda_test.resolve()
        else:
            es2panda_test = Path(config.general.static_core_root).parent.parent / "plugins" \
                            / "runtime_core" / "static_core" / "ets" / "tests" / "ets_warnings_tests"
            if not es2panda_test.exists():
                raise Exception(f'There is no path {es2panda_test}')
        self.default_list_root = es2panda_test / 'test-lists'

        if self.list_root:
            self.list_root = self.list_root
        else:
            self.list_root = path.normpath(path.join(self.default_list_root, self.name))
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.test_root = es2panda_test if self.test_root is None else self.test_root
        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        ets_flags = [
            f"--opt-level={self.config.es2panda.opt_level}",
            "--extension=ets",
            f"--arktsconfig={self.arktsconfig}"
        ] + self.config.es2panda.es2panda_args
        all_warnings = ets_flags + SystemArkTSFlags.ALL_WARNINGS.value
        boost_equality_statements = ets_flags + SystemArkTSFlags.BOOST_EQUALITY_STATEMENTS.value
        implicit_boxing_unboxing = ets_flags + SystemArkTSFlags.IMPLICIT_BOXING_UNBOXING.value
        prohibit_top_level = ets_flags + SystemArkTSFlags.PROHIBIT_TOP_LEVEL_STATEMENTS.value
        remove_async = ets_flags + SystemArkTSFlags.REMOVE_ASYNC.value
        remove_lambda = ets_flags + SystemArkTSFlags.REMOVE_LAMBDA.value
        suggest_final = ets_flags + SystemArkTSFlags.SUGGEST_FINAL.value
        suppression_tests = ets_flags + SystemArkTSFlags.SUPPRESSION_TESTS.value
        werror_tests = ets_flags + SystemArkTSFlags.WERROR_TESTS.value

        if config.es2panda.system:
            test_dirs = [
                TestDirectory("all_warnings_tests", "ets", flags=all_warnings),
                TestDirectory("boost_equal_statements_tests", "ets", flags=boost_equality_statements),
                TestDirectory("implicit_boxing_unboxing_tests", "ets", flags=implicit_boxing_unboxing),
                TestDirectory("prohibit_top_level_statements_tests", "ets", flags=prohibit_top_level),
                TestDirectory("remove_async_tests", "ets", flags=remove_async),
                TestDirectory("remove_lambda_tests", "ets", flags=remove_lambda),
                TestDirectory("suggest_final_tests", "ets", flags=suggest_final),
                TestDirectory("warnings_suppresion_tests", "ets", flags=suppression_tests),
                TestDirectory("werror_tests", "ets", flags=werror_tests),
            ]
            self.add_directories(test_dirs)

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "ets_warnings_tests"

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestETSSystem:
        test = TestETSSystem(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

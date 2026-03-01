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

import logging
from os import path
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.parser.test_js_parser import TestJSParser
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger("runner.plugins.parser.runner_js_parser")


class RunnerJSParser(RunnerJS):
    def __init__(self, config: Config) -> None:
        super().__init__(config, "parser")

        symlink_es2panda_test = Path(config.general.static_core_root) / "tools" / "es2panda" / "test"
        if symlink_es2panda_test.exists():
            es2panda_test = symlink_es2panda_test.resolve()
        else:
            es2panda_test = Path(config.general.static_core_root).parent.parent / 'ets_frontend' / 'ets2panda' / 'test'
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

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        allowed_list = self.collect_test_lists("allowed")
        self.allowed_tests = self.load_tests_from_lists(allowed_list)

        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)

        test_dirs: List[TestDirectory] = [
            TestDirectory('compiler/ts', 'ts', flags=['--extension=ts']),
            TestDirectory('compiler/ets', 'ets', flags=[
                '--extension=ets',
                f'--arktsconfig={self.arktsconfig}'
            ]),
            TestDirectory('parser/ts', 'ts', flags=['--parse-only', '--extension=ts']),
            TestDirectory('parser/as', 'ts', flags=['--parse-only', '--extension=as']),
            TestDirectory('parser/ets', 'ets', flags=[
                '--extension=ets',
                f'--arktsconfig={self.arktsconfig}'
            ]),
        ]

        if self.config.general.with_js:
            test_dirs.append(TestDirectory('compiler/js', 'js', flags=['--extension=js']))
            test_dirs.append(TestDirectory('ark_tests/parser/js', 'js', flags=['--parse-only', '--extension=js']))
            test_dirs.append(TestDirectory('parser/js', 'js', flags=['--parse-only', '--extension=js']))

        self.add_directories(test_dirs)

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "parser"

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestJSParser:
        if test_file not in self.allowed_tests:
            message = (f'JSParser is deprecated, creating new tests ({test_file}) is not allowed. '
                       'Use ASTChecker instead')
            Log.exception_and_raise(_LOGGER, message)
        test = TestJSParser(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

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

from runner.options.config import Config
from runner.runner_base import get_test_id
from runner.logger import Log
from runner.plugins.declgenparser.test_declgenparser import TestDeclgenParser
from runner.runner_js import RunnerJS
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger("runner.plugins.declgenparser.runner_declgenparser")


class RunnerDeclgenParser(RunnerJS):
    TEST_DIRS = ("cookbook_tests",)

    def __init__(self, config: Config) -> None:
        super().__init__(config, "declgenparser")

        static_core_root = Path(config.general.static_core_root)
        symlink_es2panda_test = static_core_root / "tools" / "es2panda" / "test"
        es2panda_test = symlink_es2panda_test if symlink_es2panda_test.exists() else \
            Path(config.general.static_core_root).parent.parent / 'ets_frontend' / 'ets2panda' / 'test'
        if not es2panda_test.exists():
            message = f'There is no path {es2panda_test}'
            Log.exception_and_raise(_LOGGER, message)

        declgen_root = static_core_root / "plugins" / "ets" / "tools" / "declgen_ts2sts"
        self.default_list_root = declgen_root / "test" / "test-lists"
        self.list_root = self.list_root if self.list_root else self.default_list_root
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")
        self.test_root = declgen_root / "test" if self.test_root is None else self.test_root
        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")

        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        self.add_directories(self.get_test_directories())

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "declgenparser"

    def get_test_directories(self) -> List[TestDirectory]:
        flags = ["--extension=ets"]
        return [TestDirectory(path.join(self.test_root, test_dir), "ets", flags)
                for test_dir in self.TEST_DIRS]

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestDeclgenParser:
        test = TestDeclgenParser(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

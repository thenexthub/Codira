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
from runner.plugins.hermes.test_js_hermes import TestJSHermes
from runner.plugins.hermes.util_hermes import UtilHermes
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger("runner.plugins.hermes.runner_js_hermes")


class RunnerJSHermes(RunnerJS):
    def __init__(self, config: Config) -> None:
        RunnerJS.__init__(self, config, "hermes")

        self.default_list_root = Path(config.general.static_core_root) / 'tests' / 'test-lists'
        self.list_root = self.list_root if self.list_root else path.join(self.default_list_root, self.name)
        Log.all(_LOGGER, f"LIST_ROOT set to {self.list_root}")
        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)
        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        self.util = UtilHermes(config=self.config, work_dir=self.work_dir)
        self.test_env.util = self.util
        self.test_root = self.util.generate()
        Log.summary(_LOGGER, f"TEST_ROOT reset to {self.test_root}")
        self.add_directories([TestDirectory(self.test_root, "js", [])])

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "hermes"

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestJSHermes:
        test = TestJSHermes(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

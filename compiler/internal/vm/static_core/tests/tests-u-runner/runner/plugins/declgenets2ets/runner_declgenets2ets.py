#!/usr/bin/env python3
# -- coding: utf-8 --
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

import logging
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.runner_js import RunnerJS
from runner.plugins.declgenets2ets.test_declgenets2ets import TestDeclgen
from runner.runner_base import get_test_id
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger("runner.plugins.declgenets2ets.runner_declgenets2ets")


class RunnerDeclgenEts2Ets(RunnerJS):
    def __init__(self, config: Config) -> None:
        super().__init__(config, "declgen-ets2ets")
        es2panda_test = (
            Path(config.general.static_core_root)
            .parent.parent
            .joinpath('ets_frontend', 'ets2panda', 'test', 'declgen')
        )
        self.test_root = es2panda_test
        self.list_root = Path(config.general.static_core_root).joinpath(
            "plugins", "ets", "tests", "test-lists", "declgenets2ets"
        )

        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)
        test_dirs: List[TestDirectory] = [
            TestDirectory("", "ets", flags=["--extension=ets", f'--arktsconfig={self.arktsconfig}'])
        ]
        self.add_directories(test_dirs)

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "declgen_ets2ets"

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestDeclgen:
        test = TestDeclgen(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

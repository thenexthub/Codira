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
from runner.plugins.recheck.test_recheck import TestRecheck
from runner.runner_base import get_test_id
from runner.runner_file_based import RunnerFileBased
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger('runner.plugins.recheck.runner_recheck')


class RunnerRecheck(RunnerFileBased):
    def __init__(self, config: Config) -> None:
        self.__ets_suite_name = "recheck"
        RunnerFileBased.__init__(self, config, self.__ets_suite_name)
        self.plugin_path = path.join(self.build_dir, "lib/libe2p_test_plugin_recheck.so")
        if not path.exists(self.plugin_path):
            Log.exception_and_raise(_LOGGER, f"Cannot find plugin : {self.plugin_path}", FileNotFoundError)

        symlink_es2panda_test = Path(config.general.static_core_root) / 'tools' / 'es2panda' / 'test'
        if symlink_es2panda_test.exists():
            es2panda_test = symlink_es2panda_test
        else:
            es2panda_test = Path(config.general.static_core_root).parent.parent / 'ets_frontend' / 'ets2panda' / 'test'
        self.default_list_root = es2panda_test / 'test-lists'
        self.list_root = self.list_root if self.list_root else path.join(self.default_list_root, self.name)
        Log.summary(_LOGGER, f'LIST_ROOT set to {self.list_root}')

        self.test_root = es2panda_test if self.test_root is None else self.test_root
        Log.summary(_LOGGER, f'TEST_ROOT set to {self.test_root}')

        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        flags = [
            '--extension=ets',
            '--plugins=e2p_test_plugin_recheck',
            '--exit-after-phase',
            'plugins-after-check',
            f'--arktsconfig={self.arktsconfig}',
        ]
        test_dirs: List[TestDirectory] = [
            TestDirectory('compiler/ets', 'ets', flags),
            TestDirectory('parser/ets', 'ets', flags),
            TestDirectory('runtime/ets', 'ets', flags),
            TestDirectory('ast', 'ets', flags),
        ]

        self.add_directories(test_dirs)

    @property
    def default_work_dir_root(self) -> Path:
        return Path('/tmp') / 'recheck'

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestRecheck:
        test = TestRecheck(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

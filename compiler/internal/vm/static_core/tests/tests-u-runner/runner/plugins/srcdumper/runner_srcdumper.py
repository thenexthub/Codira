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
import os
import shutil
from os import path
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.srcdumper.test_srcdumper import TestSRCDumper
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger('runner.plugins.srcdumper.runner_srcdumper')


class RunnerSRCDumper(RunnerJS):
    def __init__(self, config: Config) -> None:
        super().__init__(config, 'srcdumper')

        symlink_es2panda_test = Path(config.general.static_core_root) / 'tools' / 'es2panda' / 'test'
        if symlink_es2panda_test.exists():
            es2panda_test = symlink_es2panda_test
        else:
            es2panda_test = Path(config.general.static_core_root).parent.parent / 'ets_frontend' / 'ets2panda' / 'test'
            if not es2panda_test.exists():
                raise Exception(f'There is no path {es2panda_test}')
        self.default_list_root = es2panda_test / 'test-lists'

        self.list_root = self.list_root if self.list_root else path.join(self.default_list_root, self.name)
        Log.summary(_LOGGER, f'LIST_ROOT set to {self.list_root}')

        self.test_root = es2panda_test if self.test_root is None else self.test_root
        Log.summary(_LOGGER, f'TEST_ROOT set to {self.test_root}')

        # Set up artifacts directory for dumped source files
        self.artifacts_root = self.get_work_dir(config) / 'dumped_src'
        self.artifacts_root.mkdir(parents=True, exist_ok=True)
        Log.summary(_LOGGER, f'Artifacts directory set to {self.artifacts_root}')

        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        test_dirs = self._collect_test_directories()
        self.add_directories(test_dirs)
        self._copy_test_files_to_artifacts(test_dirs)


    @property
    def default_work_dir_root(self) -> Path:
        return Path('/tmp') / 'srcdumper'

    def get_work_dir(self, config: Config) -> Path:
        return Path(config.general.work_dir) if config.general.work_dir else self.default_work_dir_root

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestSRCDumper:
        test = TestSRCDumper(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

    def _collect_test_directories(self) -> List[TestDirectory]:
        flags = [
            '--extension=ets',
            '--output=/dev/null',
            '--exit-after-phase', 'plugins-after-parse',
            f'--arktsconfig={self.arktsconfig}'
        ]
        return [
            TestDirectory('compiler/ets', 'ets', flags),
            TestDirectory('parser/ets', 'ets', flags),
            TestDirectory('runtime/ets', 'ets', flags),
            TestDirectory('ast', 'ets', flags),
            TestDirectory('srcdump', 'ets', flags),
        ]

    def _copy_test_files_to_artifacts(self, test_dirs: List[TestDirectory]) -> None:
        """Copy test files to artifacts directory while preserving directory structure."""
        for test_dir in test_dirs:
            source_dir = Path(self.test_root) / test_dir.directory
            if not source_dir.exists():
                Log.summary(_LOGGER, f'Source directory {source_dir} does not exist, skipping')
                continue

            target_dir = self.artifacts_root / test_dir.directory
            target_dir.mkdir(parents=True, exist_ok=True)

            # Get all .ets files in the directory
            ets_files = [
                (Path(root) / file, source_dir)
                for root, _, files in os.walk(source_dir)
                for file in files
                if file.endswith('.ets')
            ]

            # Copy each file
            for source_file, source_dir in ets_files:
                rel_path = source_file.relative_to(source_dir)
                target_file = target_dir / rel_path
                target_file.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source_file, target_file)
                Log.summary(_LOGGER, f'Copied {source_file} to {target_file}')

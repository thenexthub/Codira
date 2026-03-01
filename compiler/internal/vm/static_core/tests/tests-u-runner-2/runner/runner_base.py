#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

from abc import ABC, abstractmethod
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
from pathlib import Path

import pytz
from tqdm import tqdm

from runner.logger import Log
from runner.options.config import Config
from runner.test_base import Test
from runner.utils import correct_path

_LOGGER = Log.get_logger(__file__)


def run_test(pair: tuple[Test, int]) -> Test:
    return pair[0].run(pair[1])


class Runner(ABC):
    def __init__(self, config: Config, name: str) -> None:
        # Roots:
        # directory where test files are located - it's either set explicitly to the absolute value
        # or the current folder (where this python file is located!) parent
        self.test_root: Path | None = None
        # directory where list files (files with list of ignored, excluded, and other tests) are located
        # it's either set explicitly to the absolute value or
        # the current folder (where this python file is located!) parent
        self.list_root: Path | None = config.test_suite.list_root
        if self.list_root is not None:
            _LOGGER.summary(f"LIST_ROOT set to {self.list_root}")

        # runner init time
        self.start_time = datetime.now(pytz.UTC)

        self.config = config
        self.name: str = name

        # Lists:
        # excluded test is a test what should not be loaded and should be tried to run
        # excluded_list: either absolute path or path relative from list_root to the file with the list of such tests
        self.excluded_lists: list[str] = []
        self.excluded_tests: set[Path] = set([])
        # ignored test is a test what should be loaded and executed, but its failure should be ignored
        # ignored_list: either absolute path or path relative from list_root to the file with the list of such tests
        # aka: kfl stands for `known failures list`
        self.ignored_lists: list[str] = []
        self.ignored_tests: set[str] = set([])
        # list of file names, each is a name of a test. Every test should be executed
        # So, it can contain ignored tests, but cannot contain excluded tests
        self.tests: set[Test] = set([])
        # list of results of every executed test
        self.results: list[Test] = []
        # name of file with a list of only tests what should be executed
        # if it's specified other tests are not executed
        self.explicit_list = correct_path(self.list_root, config.test_suite.test_lists.explicit_list) \
            if config.test_suite.test_lists.explicit_list is not None and self.list_root is not None \
            else None
        # name of the single test file in form of a relative path from test_root what should be executed
        # if it's specified other tests are not executed even if test_list is set
        self.explicit_test = config.test_suite.test_lists.explicit_file

        # Counters:
        # failed + ignored + passed + excluded_after = len of all executed tests
        # failed + ignored + passed + excluded_after + excluded = len of full set of tests
        self.failed = 0
        self.ignored = 0
        self.passed = 0
        self.excluded = 0
        # Test chosen to execute can detect itself as excluded one
        self.excluded_after = 0
        self.update_excluded = config.test_suite.test_lists.update_excluded

    @staticmethod
    def add_tests_param(tests: set[Test], repeat: int) -> list[tuple[Test, int]]:
        return [(test, repeat) for test in tests]

    @abstractmethod
    def summarize(self) -> int:
        pass

    @abstractmethod
    def create_coverage_html(self) -> None:
        pass

    def run_threads(self, repeat: int) -> None:
        _LOGGER.all(f"--Start test running with {self.config.general.processes} threads")
        with ThreadPoolExecutor(self.config.general.processes) as executor:
            results = executor.map(
                run_test,
                self.add_tests_param(self.tests, repeat))
            if self.config.general.show_progress:
                results = tqdm(results, total=len(self.tests))
            self.results = list(results)

    def before_suite(self) -> None:
        pass

    def after_suite(self) -> None:
        pass

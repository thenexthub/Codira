#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import fnmatch
import logging
import multiprocessing
import re
import argparse
import sys
from unittest import TestCase
from abc import abstractmethod, ABC
from collections import Counter
from datetime import datetime
from glob import glob
from os import path
from pathlib import Path
from typing import List, Set, Tuple, Optional

import pytz
from tqdm import tqdm

from runner.chapters import Chapters
from runner.enum_types.configuration_kind import ConfigurationKind
from runner.options.cli_args_wrapper import CliArgsWrapper
from runner.plugins.ets.test_ets import TestETS
from runner.logger import Log
from runner.options.config import Config
from runner.test_base import Test
from runner.utils import get_group_number
from runner.enum_types.test_directory import TestDirectory

CONST_COMMENT = ["#"]
TEST_COMMENT_EXPR = re.compile(r"^\s*(?P<test>[^# ]+)?(\s*#\s*(?P<comment>.+))?", re.MULTILINE)


def load_test_from_list(test_root: str, line: str, directory: Optional[str] = None) -> Optional[str]:
    test, _ = get_test_and_comment_from_line(line.strip(" \n"))
    if test is None:
        return None
    extra_dir_check = (directory is None) or (directory is not None and test.startswith(str(directory)))
    if extra_dir_check:
        test_file = path.normpath(path.join(test_root, test))
        if path.exists(test_file):
            return str(test_file)
    return None


def load_list(test_root: str, test_list_path: str, directory: Optional[str] = None) -> List[str]:
    result: List[str] = []
    if not path.exists(test_list_path):
        return result

    with open(test_list_path, 'r', encoding="utf-8") as file:
        for line in file:
            test_file = load_test_from_list(test_root, line, directory)
            if test_file:
                result.append(path.normpath(test_file))

    return result


def get_test_id(file: str, start_directory: Path) -> str:
    relpath = Path(file).relative_to(start_directory)
    return str(relpath)


def get_test_and_comment_from_line(line: str) -> Tuple[Optional[str], Optional[str]]:
    line_parts = TEST_COMMENT_EXPR.search(line)
    if line_parts:
        return line_parts["test"], line_parts["comment"]
    return None, None


def correct_path(root: Path, test_list: str) -> str:
    return path.abspath(test_list) if path.exists(test_list) else path.normpath(path.join(root, test_list))


_LOGGER = logging.getLogger("runner.runner_base")

# pylint: disable=invalid-name,global-statement
worker_cli_wrapper_args: Optional[argparse.Namespace] = None


def init_worker(shared_args: argparse.Namespace) -> None:
    global worker_cli_wrapper_args
    worker_cli_wrapper_args = shared_args


def run_test(test: Test) -> Test:
    CliArgsWrapper.args = worker_cli_wrapper_args
    return test.run()


class Runner(ABC):
    def __init__(self, config: Config, name: str) -> None:
        # TODO(vpukhov): adjust es2panda path
        default_ets_arktsconfig = path.normpath(path.join(
            config.general.build,
            "tools", "es2panda", "generated", "arktsconfig.json"
        ))
        if not path.exists(default_ets_arktsconfig):
            default_ets_arktsconfig = path.normpath(path.join(
                config.general.build,
                "gen",  # for GN build
                "tools", "es2panda", "generated", "arktsconfig.json"
            ))

        # Roots:
        # directory where test files are located - it's either set explicitly to the absolute value
        # or the current folder (where this python file is located!) parent
        self.test_root = config.general.test_root
        if self.test_root is not None:
            Log.summary(_LOGGER, f"TEST_ROOT set to '{self.test_root}'")
        # directory where list files (files with list of ignored, excluded, and other tests) are located
        # it's either set explicitly to the absolute value or
        # the current folder (where this python file is located!) parent
        self.default_list_root = Path(config.general.static_core_root) / 'plugins' / 'ets' / 'tests' / 'test-lists'
        self.list_root = config.general.list_root
        if self.list_root is not None:
            Log.summary(_LOGGER, f"LIST_ROOT set to '{self.list_root}'")

        # runner init time
        self.start_time = datetime.now(pytz.UTC)
        # root directory containing bin folder with binary files
        self.build_dir = config.general.build
        self.arktsconfig = config.es2panda.arktsconfig \
            if config.es2panda.arktsconfig is not None \
            else default_ets_arktsconfig

        self.config = config
        self.name: str = name

        # Lists:
        # excluded test is a test what should not be loaded and should be tried to run
        # excluded_list: either absolute path or path relative from list_root to the file with the list of such tests
        self.excluded_lists: List[str] = []
        self.excluded_tests: Set[str] = set([])
        # ignored test is a test what should be loaded and executed, but its failure should be ignored
        # ignored_list: either absolute path or path relative from list_root to the file with the list of such tests
        # aka: kfl = known failures list
        self.ignored_lists: List[str] = []
        self.ignored_tests: Set[str] = set([])
        # list of file names, each is a name of a test. Every test should be executed
        # So, it can contain ignored tests, but cannot contain excluded tests
        self.tests: Set[Test] = set([])
        # list of results of every executed test
        self.results: List[Test] = []
        # name of file with a list of only tests what should be executed
        # if it's specified other tests are not executed
        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)
        # name of the single test file in form of a relative path from test_root what should be executed
        # if it's specified other tests are not executed even if test_list is set
        self.explicit_test = config.test_lists.explicit_file

        # Counters:
        # failed + ignored + passed + excluded_after = len of all executed tests
        # failed + ignored + passed + excluded_after + excluded = len of full set of tests
        self.failed = 0
        self.ignored = 0
        self.passed = 0
        self.excluded = 0
        # Test chosen to execute can detect itself as excluded one
        self.excluded_after = 0
        self.update_excluded = config.test_lists.update_excluded

        self.conf_kind = Runner.detect_conf(config)

    @staticmethod
    # pylint: disable=too-many-return-statements
    def detect_conf(config: Config) -> ConfigurationKind:
        if config.ark_aot.enable:
            is_aot_full = len([
                arg for arg in config.ark_aot.aot_args
                if "--compiler-inline-full-intrinsics=true" in arg
            ]) > 0
            if is_aot_full:
                return ConfigurationKind.AOT_FULL
            return ConfigurationKind.AOT

        if config.ark.jit.enable:
            return ConfigurationKind.JIT

        if config.ark.interpreter_type:
            return ConfigurationKind.OTHER_INT

        if config.quick.enable:
            return ConfigurationKind.QUICK

        return ConfigurationKind.INT

    @staticmethod
    def _search_duplicates(original: List[str], kind: str) -> None:
        main_counter = Counter(original)
        dupes = [test for test, frequency in main_counter.items() if frequency > 1]
        if len(dupes) > 0:
            Log.summary(_LOGGER, f"There are {len(dupes)} duplicates in {kind} lists.")
            for test in dupes:
                Log.short(_LOGGER, f"\t{test}")
        elif len(original) > 0:
            Log.summary(_LOGGER, f"No duplicates found in {kind} lists.")

    def recalculate_explicit_list(self, explicit_list: str) -> Optional[str]:
        return correct_path(self.list_root, explicit_list) \
            if explicit_list is not None and self.list_root is not None \
            else None

    @abstractmethod
    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> Test:
        pass

    @abstractmethod
    def summarize(self) -> int:
        pass

    @abstractmethod
    def create_coverage_html(self) -> None:
        pass

    @abstractmethod
    def clear_gcda_files(self) -> None:
        pass

    def run(self) -> None:
        Log.all(_LOGGER, "Start test running")
        with multiprocessing.Pool(processes=self.config.general.processes,
                                  initializer=init_worker, initargs=(CliArgsWrapper.args,)) as pool:
            results = pool.imap_unordered(run_test, self.tests, chunksize=self.config.general.chunksize)
            if self.config.general.show_progress:
                results = tqdm(results, total=len(self.tests))
            self.results = list(results)
            pool.close()
            pool.join()

    def load_tests_from_lists(self, lists: List[str], directory: Optional[str] = None) -> List[str]:
        tests = []
        for list_name in lists:
            list_path = correct_path(self.list_root, list_name)
            Log.summary(_LOGGER, f"Loading tests from the list '{list_path}'")
            TestCase().assertTrue(self.test_root, "TEST_ROOT not set to correct value")
            tests.extend(load_list(self.test_root, list_path, directory))
        return tests

    # Read excluded_lists and load list of excluded tests
    def load_excluded_tests(self) -> None:
        excluded_tests = self.load_tests_from_lists(self.excluded_lists)

        if self.config.test_lists.groups.quantity > 1:
            groups = self.config.test_lists.groups.quantity
            n_group = self.config.test_lists.groups.number
            n_group = n_group if n_group <= groups else groups
            excluded_tests_in_group = {
                test for test in excluded_tests
                if get_group_number(Path(test).relative_to(self.test_root).as_posix(), groups) == n_group}
            self.excluded_tests.update(excluded_tests_in_group)
        else:
            self._search_duplicates(excluded_tests, "excluded")
            self.excluded_tests.update(excluded_tests)

        self.excluded = len(self.excluded_tests)

    # Read ignored_lists and load list of ignored tests
    def load_ignored_tests(self) -> None:
        ignored_tests = self.load_tests_from_lists(self.ignored_lists)
        self.ignored_tests.update(ignored_tests)
        self._search_duplicates(ignored_tests, "ignored")

    def add_directories(self, test_dirs: List[TestDirectory]) -> None:
        for test_dir in test_dirs:
            self.add_directory(test_dir)
        if not self.tests:
            Log.default(
                _LOGGER,
                "No tests have been loaded. "
                "Check values of filtering options: --filter, --groups and --group-number, "
                "--test-list, --chapter")
            sys.exit(-1)

    # Browse the directory, search for files with the specified extension
    # and add them as tests
    def add_directory(self, test_dir: TestDirectory) -> None:
        directory, extension, flags = test_dir
        directory = str(directory)
        Log.all(_LOGGER, f"Loading tests from the directory '{directory}'")
        test_files = []
        if self.explicit_test is not None:
            if self.explicit_test.startswith(directory) or path.isdir(directory):
                test_files.extend([correct_path(self.test_root, self.explicit_test)])
        elif self.explicit_list is not None:
            extra_dir = None if directory == self.test_root or directory.startswith(str(self.test_root)) else directory
            test_files.extend(self.load_tests_from_lists([self.explicit_list], extra_dir))
        else:
            if not self.config.test_lists.skip_test_lists:
                self.load_excluded_tests()
                self.load_ignored_tests()
            if not path.exists(directory):
                directory = str(path.normpath(path.join(self.test_root, directory)))
            test_files.extend(self.__load_test_files(directory, extension))  # type: ignore

        self._search_both_excluded_and_ignored_tests()
        self._search_not_used_ignored(test_files)

        all_tests = {self.create_test(path.normpath(test), flags, test in self.ignored_tests)  # type: ignore
                     for test in test_files}
        not_tests = {t for t in all_tests if isinstance(t, TestETS) and not t.is_valid_test}
        valid_tests = all_tests - not_tests

        if self.config.test_lists.groups.quantity > 1:
            groups = self.config.test_lists.groups.quantity
            n_group = self.config.test_lists.groups.number
            n_group = n_group if n_group <= groups else groups
            valid_tests = {test for test in valid_tests if get_group_number(test.test_id, groups) == n_group}

        self.tests.update(valid_tests)
        Log.default(_LOGGER, f"Loaded {len(valid_tests)} valid tests from the folder '{directory}'. "
                             f"Excluded {len(self.excluded_tests)} tests are not loaded.")

    def _search_both_excluded_and_ignored_tests(self) -> None:
        already_excluded = [test for test in self.ignored_tests if test in self.excluded_tests]
        if not already_excluded:
            return
        Log.summary(_LOGGER, f"Found {len(already_excluded)} tests present both in excluded and ignored test "
                             f"lists.")
        for test in already_excluded:
            Log.all(_LOGGER, f"\t{test}")
            self.ignored_tests.remove(test)

    def _search_not_used_ignored(self, found_tests: List[str]) -> None:
        ignored_absent = [test for test in self.ignored_tests if test not in found_tests]
        if ignored_absent:
            Log.summary(_LOGGER, f"Found {len(ignored_absent)} tests in ignored lists but absent on the file system:")
            for test in ignored_absent:
                Log.summary(_LOGGER, f"\t{test}")
        else:
            Log.short(_LOGGER, "All ignored tests are found on the file system")

    def __load_test_files(self, directory: str, extension: str) -> List[str]:
        if self.config.test_lists.filter != "*" and self.config.test_lists.groups.chapters:
            Log.exception_and_raise(
                _LOGGER,
                "Incorrect configuration: specify either filter or chapter options"
            )
        test_files: List[str] = []
        excluded: List[str] = list(self.excluded_tests)[:]
        glob_expression = path.join(directory, f"**/*.{extension}")
        test_files.extend(fnmatch.filter(
            glob(glob_expression, recursive=True),
            path.normpath(path.join(directory, self.config.test_lists.filter))
        ))
        if self.config.test_lists.groups.chapters:
            test_files = self.__filter_by_chapters(directory, test_files, extension)
        return [
            test for test in test_files
            if self.update_excluded or test not in excluded
        ]

    def __filter_by_chapters(self, base_folder: str, files: List[str], extension: str) -> List[str]:
        test_files: Set[str] = set()
        chapters: Chapters = self.__parse_chapters()
        for chapter in self.config.test_lists.groups.chapters:
            test_files.update(chapters.filter_by_chapter(chapter, base_folder, files, extension))
        return list(test_files)

    def __parse_chapters(self) -> Chapters:
        chapters: Optional[Chapters] = None
        if path.isfile(self.config.test_lists.groups.chapters_file):
            chapters = Chapters(self.config.test_lists.groups.chapters_file)
        else:
            corrected_chapters_file = correct_path(self.list_root, self.config.test_lists.groups.chapters_file)
            if path.isfile(corrected_chapters_file):
                chapters = Chapters(corrected_chapters_file)
            else:
                Log.exception_and_raise(
                    _LOGGER,
                    f"Not found either '{self.config.test_lists.groups.chapters_file}' or "
                    f"'{corrected_chapters_file}'", FileNotFoundError)
        return chapters

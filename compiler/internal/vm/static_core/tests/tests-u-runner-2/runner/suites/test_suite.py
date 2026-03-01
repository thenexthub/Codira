#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import fnmatch
import os
import re
import shutil
import subprocess
from collections import Counter
from functools import cached_property
from glob import glob
from os import path
from pathlib import Path
from typing import ClassVar, cast

from runner.chapters import Chapters
from runner.common_exceptions import (
    FileNotFoundException,
    InvalidConfiguration,
    InvalidGTestNameException,
    TestNotExistException,
)
from runner.enum_types.params import TestEnv
from runner.enum_types.qemu import QemuKind
from runner.extensions.flows.test_flow_registry import ITestFlow, workflow_registry
from runner.extensions.suites.test_suite_registry import ITestSuite
from runner.logger import Log
from runner.options.options import IOptions
from runner.options.options_collections import CollectionsOptions
from runner.suites.preparation_step import CopyStep, CustomGeneratorTestPreparationStep, JitStep, TestPreparationStep
from runner.suites.step_utils import StepUtils
from runner.suites.test_lists import TestLists
from runner.utils import correct_path, get_group_number, get_test_id, is_executable_file

_LOGGER = Log.get_logger(__file__)


class TestSuite(ITestSuite):
    CONST_COMMENT: ClassVar[str] = "#"
    TEST_COMMENT_EXPR = re.compile(r"^\s*(?P<test>[^# ]+)?(\s*#\s*(?P<comment>.+))?", re.MULTILINE)

    def __init__(self, test_env: TestEnv) -> None:
        self._test_env = test_env
        self.__suite_name = test_env.config.test_suite.suite_name
        self.__work_dir = test_env.work_dir
        self._list_root = Path(test_env.config.test_suite.list_root)

        self.config = test_env.config

        self.jit_repeats: int = int(repeats) \
            if (repeats := self.config.workflow.get_parameter("num-repeats")) is not None else 0
        self._preparation_steps: list[TestPreparationStep] = self.set_preparation_steps()
        self._collections_parameters = self.__get_collections_parameters()

        self._explicit_file: Path | None = None
        self._explicit_list: Path | None = None
        self._test_lists = TestLists(self._list_root, self._test_env, self.jit_repeats)
        self._test_lists.collect_excluded_test_lists(test_name=self.__suite_name)
        self._test_lists.collect_ignored_test_lists(test_name=self.__suite_name)
        self.excluded = 0
        self.ignored_tests: list[Path] = []
        self.ignored_tests_with_failures: dict[Path, str] = {}
        self.excluded_tests: list[Path] = []
        self.workflow_kind = self.config.workflow.workflow_type

    @property
    def test_root(self) -> Path:
        return self.__work_dir.gen

    @staticmethod
    def __load_list(test_root: Path, test_list_path: Path, prefixes: list[str]) -> tuple[list[Path], list[Path]]:
        result: list[Path] = []
        not_found: list[Path] = []
        if not test_list_path.exists():
            return result, not_found

        with os.fdopen(os.open(test_list_path, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_handler:
            for line in file_handler:
                is_found, test_path = TestSuite.__load_line(line, test_root, prefixes)
                if is_found and test_path is not None:
                    result.append(test_path)
                elif not is_found and test_path is not None:
                    not_found.append(test_path)
        return result, not_found

    @staticmethod
    def __load_line(line: str, test_root: Path, prefixes: list[str]) -> tuple[bool, Path | None]:
        test, _ = TestSuite._get_test_and_comment_from_line(line.strip(" \n"))
        if test is None:
            return False, None
        test_path = test_root / test
        if test_path.exists():
            return True, test_path
        is_found, prefixed_test_path = TestSuite.__load_line_with_prefix(test_root, prefixes, test)
        if prefixed_test_path is not None:
            return is_found, prefixed_test_path
        return False, test_path

    @staticmethod
    def __load_line_with_prefix(test_root: Path, prefixes: list[str], test: str) -> tuple[bool, Path | None]:
        for prefix in prefixes:
            test_path = test_root / prefix
            if test_path.is_file() and test_path.name == test:
                return True, test_path
            if test_path.is_dir():
                test_path = test_path / test
                if test_path.exists():
                    return True, test_path
        return False, None

    @staticmethod
    def _get_test_and_comment_from_line(line: str) -> tuple[str | None, str | None]:
        line_parts = TestSuite.TEST_COMMENT_EXPR.search(line)
        if line_parts:
            return line_parts["test"], line_parts["comment"]
        return None, None

    @staticmethod
    def _search_duplicates(original: list[Path], kind: str) -> None:
        main_counter = Counter([str(test) for test in original])
        dupes = [test for test, frequency in main_counter.items() if frequency > 1]
        if len(dupes) > 0:
            _LOGGER.summary(f"There are {len(dupes)} duplicates in {kind} lists.")
            for test in dupes:
                Log.short(_LOGGER, f"\t{test}")
        elif len(original) > 0:
            _LOGGER.summary(f"No duplicates found in {kind} lists.")

    @staticmethod
    def __is_path_excluded(collection: CollectionsOptions, tested_path: Path) -> bool:
        excluded = [excl for excl in collection.exclude if tested_path.as_posix().endswith(excl)]
        return len(excluded) > 0

    @staticmethod
    def _comment_has_failure(comment: str | None) -> bool:
        return bool(TestSuite._get_failure_text(comment))

    @staticmethod
    def _get_failure_text(comment: str | None) -> str:
        pattern = r'^#\d+\s+.*?@@Failure:\s*(.+?)@@.*$'
        if comment is not None:
            match = re.match(pattern, comment)
            return match.group(1) if match else ""
        return ""

    @cached_property
    def name(self) -> str:
        return self.__suite_name

    @cached_property
    def list_root(self) -> Path:
        return self._list_root

    @property
    def explicit_list(self) -> Path | None:
        return self._explicit_list

    @cached_property
    def explicit_file(self) -> Path | None:
        return self._explicit_file

    def process(self, force_generate: bool) -> list[ITestFlow]:
        raw_set = self._get_raw_set(force_generate)
        self._explicit_file = self._set_explicit_file()
        self._explicit_list = self._set_explicit_list()
        executed_set = self._get_executed_files(raw_set)
        self._search_both_excluded_and_ignored_tests()
        executed_set = self._get_by_groups(executed_set)
        tests = self._create_tests(executed_set)
        Log.default(_LOGGER, f"Loaded {len(tests)} valid tests from the folder '{self.test_root}'. "
                             f"Excluded {self.excluded} tests are not loaded.")
        return tests

    def set_preparation_steps(self) -> list[TestPreparationStep]:
        steps: list[TestPreparationStep] = []
        for collection in self.config.test_suite.collections:
            if collection.generator_script is not None or collection.generator_class is not None:
                steps.append(CustomGeneratorTestPreparationStep(
                    test_source_path=collection.test_root / collection.name,
                    test_gen_path=self.test_root / collection.name,
                    config=self.config,
                    collection=collection,
                    extension=self.config.test_suite.extension(collection)
                ))
                copy_source_path = self.test_root / collection.name
                real_source_path = collection.test_root / collection.name
            else:
                copy_source_path = collection.test_root / collection.name
                real_source_path = copy_source_path
            if not real_source_path.exists():
                error = (f"Source path '{real_source_path}' does not exist! "
                         f"Cannot process the collection '{collection.name}'")
                _LOGGER.default(error)
                continue
            extension = self.config.test_suite.extension(collection)
            with_js = self.config.test_suite.with_js(collection)
            if (extension == "js" and with_js) or extension != "js":
                steps.extend(self.__add_copy_steps(collection, copy_source_path, extension))
            if self.jit_repeats > 0:
                steps.extend(self.__add_jit_steps(collection, copy_source_path, extension))
        return steps

    def __add_copy_steps(self, collection: CollectionsOptions, copy_source_path: Path, extension: str) \
            -> list[TestPreparationStep]:
        steps: list[TestPreparationStep] = []
        if collection.exclude:
            for file_path in copy_source_path.iterdir():
                if self.__is_path_excluded(collection, file_path):
                    continue
                steps.append(CopyStep(
                    test_source_path=file_path,
                    test_gen_path=self.test_root / collection.name / file_path.name,
                    config=self.config,
                    collection=collection,
                    extension=extension
                ))
        elif copy_source_path != self.test_root / collection.name:
            steps.append(CopyStep(
                test_source_path=copy_source_path,
                test_gen_path=self.test_root / collection.name,
                config=self.config,
                collection=collection,
                extension=extension
            ))
        return steps

    def __add_jit_steps(self, collection: CollectionsOptions, copy_source_path: Path, extension: str) \
            -> list[TestPreparationStep]:
        steps: list[TestPreparationStep] = []
        if collection.exclude:
            for file_path in copy_source_path.iterdir():
                if self.__is_path_excluded(collection, file_path):
                    continue
                steps.append(JitStep(
                    test_source_path=file_path,
                    test_gen_path=self.test_root / collection.name / file_path.name,
                    config=self.config,
                    collection=collection,
                    extension=extension,
                    num_repeats=self.jit_repeats
                ))
        else:
            steps.append(JitStep(
                test_source_path=copy_source_path,
                test_gen_path=self.test_root / collection.name,
                config=self.config,
                collection=collection,
                extension=extension,
                num_repeats=self.jit_repeats
            ))
        return steps

    def _get_raw_set(self, force_generate: bool) -> list[Path]:
        util = StepUtils()
        tests: list[Path] = []
        if not force_generate and util.are_tests_generated(self.test_root):
            _LOGGER.all(f"Reused earlier generated tests from {self.test_root}")
            return self.__load_generated_test_files()

        if self.test_root.exists():
            _LOGGER.all(f"INFO: {self.test_root.absolute()!s} already exist. WILL BE CLEANED")
            shutil.rmtree(self.test_root)

        for step in self._preparation_steps:
            tests.extend(step.transform(force_generate))
        tests = list(set(tests))

        if len(tests) == 0:
            raise InvalidConfiguration("No tests loaded to execution")

        util.create_report(self.test_root, tests)

        return tests

    def __load_generated_test_files(self) -> list[Path]:
        tests = []
        for collection in self.config.test_suite.collections:
            extension = self.config.test_suite.extension(collection)
            glob_expression = path.join(self.test_root, collection.name, f"**/*.{extension}")
            tests.extend(glob(glob_expression, recursive=True))
        return self.__load_test_files([Path(test) for test in set(tests)])

    def __get_explicit_test_path(self, test_id: str) -> Path | None:

        for collection in self.config.test_suite.collections:
            if test_id.startswith(collection.name):
                test_path: Path = correct_path(self.test_root, test_id)
                if test_path.exists():
                    return test_path
                break
            new_test_id = str(os.path.join(collection.name, test_id))
            test_path = correct_path(self.test_root, new_test_id)
            if test_path and test_path.exists():
                return test_path
        return None

    def _get_executed_files(self, raw_test_files: list[Path]) -> list[Path]:
        """
        Browse the directory, search for files with the specified extension
        """
        _LOGGER.summary(f"Loading tests from the directory {self.test_root}")
        test_files: list[Path] = []
        if self.explicit_file is not None:
            test_files.append(self.explicit_file)
        elif self.explicit_list is not None:
            test_files.extend(self._load_tests_from_lists([self.explicit_list]))
        else:
            if not self.config.test_suite.test_lists.skip_test_lists:
                self.excluded_tests = self._load_excluded_tests()
                self.ignored_tests = self._load_ignored_tests()
                self.ignored_tests_with_failures = self._get_tests_from_ignore_with_failures(
                    self._test_lists.ignored_lists)
            test_files.extend(self.__load_test_files(raw_test_files))
        return test_files

    def _get_by_groups(self, raw_test_files: list[Path]) -> list[Path]:
        if self.config.test_suite.groups.quantity > 1:
            filtered_tests = {test for test in raw_test_files if self.__in_group_number(test)}
            return list(filtered_tests)
        return raw_test_files

    def __get_n_group(self) -> int:
        groups = self.config.test_suite.groups.quantity
        n_group = self.config.test_suite.groups.number
        return n_group if n_group <= groups else groups

    def __in_group_number(self, test: Path) -> bool:
        groups = self.config.test_suite.groups.quantity
        n_group = self.__get_n_group()
        return get_group_number(str(test.relative_to(self.test_root)), groups) == n_group

    def _create_test(self, test_file: Path, is_ignored: bool) -> ITestFlow:
        test_id = get_test_id(test_file, self.test_root)
        coll_name = self._get_coll_name(test_id)
        params = self._collections_parameters.get(coll_name, {}) if coll_name is not None else {}
        test = workflow_registry.create(self.workflow_kind, test_env=self._test_env, test_path=test_file,
                                        params=IOptions(params), test_id=test_id)
        test.ignored = is_ignored
        if is_ignored and test.path in self.ignored_tests_with_failures:
            test.last_failure = self.ignored_tests_with_failures[test.path]

        return test

    def _get_coll_name(self, test_id: str) -> str | None:
        coll_names = [name for name in self._collections_parameters if test_id.startswith(name)]
        if len(coll_names) == 1:
            return coll_names[0]
        weights = {len(name): name for name in coll_names}
        if weights:
            name = max(weights.keys())
            return weights[name]
        return None

    def _create_tests(self, raw_test_files: list[Path]) -> list[ITestFlow]:
        all_tests: set[ITestFlow] = set()
        for test in raw_test_files:
            all_tests.add(self._create_test(test, test in self.ignored_tests))

        not_tests = {t for t in all_tests if not t.is_valid_test}
        valid_tests = all_tests - not_tests
        if self.config.test_suite.skip_compile_only_neg:
            compile_only_tests_neg = {t for t in all_tests if t.is_negative_compile}
            valid_tests = valid_tests - compile_only_tests_neg
        _LOGGER.all(f"Loaded {len(valid_tests)} tests")

        return list(valid_tests)

    def _get_test_root_pattern(self, mask: str) -> re.Pattern:
        return re.compile(f"{self.test_root / mask}")

    def __load_test_files(self, raw_test_files: list[Path]) -> list[Path]:
        if self.config.test_suite.filter != "*" and self.config.test_suite.groups.chapters:
            raise InvalidConfiguration(
                "Incorrect configuration: specify either filter or chapter options"
            )
        test_files: list[Path] = raw_test_files
        excluded: list[Path] = list(self.excluded_tests)[:]
        if self.config.test_suite.groups.chapters:
            test_files = self.__filter_by_chapters(self.test_root, test_files)
        mask = self.config.test_suite.filter.replace(".", r"\.").replace("*", ".*")
        coll_names = "|".join(self._collections_parameters.keys())
        mask = f"({coll_names})?/{mask}" if coll_names else mask
        pattern = self._get_test_root_pattern(mask)
        return [test for test in test_files if test not in excluded and pattern.search(str(test))]

    def __filter_by_chapters(self, base_folder: Path, files: list[Path]) -> list[Path]:
        test_files: set[Path] = set()
        chapters: Chapters = self.__parse_chapters()
        for chapter in self.config.test_suite.groups.chapters:
            test_files.update(chapters.filter_by_chapter(chapter, base_folder, files))
        return list(test_files)

    def __parse_chapters(self) -> Chapters:
        chapters: Chapters | None = None
        if path.isfile(self.config.test_suite.groups.chapters_file):
            chapters = Chapters(self.config.test_suite.groups.chapters_file)
        else:
            corrected_chapters_file = correct_path(self.list_root,
                                                   self.config.test_suite.groups.chapters_file)
            if path.isfile(corrected_chapters_file):
                chapters = Chapters(corrected_chapters_file)
            else:
                raise FileNotFoundException(
                    f"Not found either '{self.config.test_suite.groups.chapters_file}' or "
                    f"'{corrected_chapters_file}'")
        return chapters

    def _set_explicit_file(self) -> Path | None:
        explicit_file = self.config.test_suite.test_lists.explicit_file
        if explicit_file is not None and self.list_root is not None:
            test_path = self.__get_explicit_test_path(explicit_file)
            if test_path and test_path.exists():
                return test_path
            raise TestNotExistException(f"Test '{explicit_file}' does not exist")
        return None

    def _set_explicit_list(self) -> Path | None:
        if self.config.test_suite.test_lists.explicit_list is not None and self.list_root is not None:
            return correct_path(self.list_root, self.config.test_suite.test_lists.explicit_list)
        return None

    def _load_tests_from_lists(self, lists: list[Path]) -> list[Path]:
        tests = []
        any_not_found = False
        report = []
        for list_path in lists:
            _LOGGER.default(f"Loading tests from the list {list_path}")
            prefixes: list[str] = []
            if len(self.config.test_suite.collections) > 1:
                prefixes = [coll.name for coll in self.config.test_suite.collections]
            loaded, not_found = self.__load_list(self.test_root, list_path, prefixes)
            tests.extend(loaded)
            if not_found:
                any_not_found = True
                report.append(f"List '{list_path}': following tests are not found on the file system:")
                for test in not_found:
                    report.append(str(test))
        if any_not_found:
            _LOGGER.summary("\n".join(report))
        return tests

    def _load_excluded_tests(self) -> list[Path]:
        """
        Read excluded_lists and load list of excluded tests
        """
        excluded_tests = self._get_by_groups(self._load_tests_from_lists(self._test_lists.excluded_lists))
        self._search_duplicates(excluded_tests, "excluded")
        self.excluded = len(set(excluded_tests))
        return excluded_tests

    def _load_ignored_tests(self) -> list[Path]:
        """
        Read ignored_lists and load list of ignored tests
        """
        ignored_tests = self._load_tests_from_lists(self._test_lists.ignored_lists)
        self._search_duplicates(ignored_tests, "ignored")
        return ignored_tests

    def _search_both_excluded_and_ignored_tests(self) -> None:
        already_excluded = [test for test in self.ignored_tests if test in self.excluded_tests]
        if not already_excluded:
            return
        _LOGGER.summary(f"Found {len(already_excluded)} tests present both "
                        "in excluded and ignored test lists.")
        for test in already_excluded:
            _LOGGER.all(f"\t{test}")
            self.ignored_tests.remove(test)

    def __get_collections_parameters(self) -> dict[str, dict[str, list | str]]:
        result: dict[str, dict[str, list | str]] = {}
        for collection in self._test_env.config.test_suite.collections:
            result[collection.name] = collection.parameters
        return result

    def _get_tests_from_ignore_with_failures(self, lists: list[Path]) -> dict[Path, str]:
        tests: dict[Path, str] = {}
        for list_path in lists:
            tests_from_list = self._get_tests_with_comment(list_path)
            tests = {**tests, **tests_from_list}

        tests = {key: val for key, val in tests.items() if TestSuite._comment_has_failure(val)}
        tests = {key: TestSuite._get_failure_text(val) for key, val in tests.items()}
        return tests

    def _get_tests_with_comment(self, list_path: Path) -> dict[Path, str]:
        tests: dict[Path, str] = {}
        comment_lines: list[str] = []
        prev_line_was_comment = False

        with open(list_path, encoding="utf-8") as f:
            for line in f:
                line = line.rstrip('\n').strip()
                if not line:
                    continue

                if line.startswith("#"):
                    if not prev_line_was_comment:
                        comment_lines = []
                    comment_lines.append(line)
                    prev_line_was_comment = True
                    continue

                prev_line_was_comment = False

                if not line.endswith(".ets"):
                    continue

                test_path = self.test_root / line
                if test_path.exists():
                    tests[test_path] = "\n".join(comment_lines)

        return tests


class GTestSuite(TestSuite):
    TEST_FILE = "file_name"
    TEST_CLASS_NAME = "cls_name"
    TEST_NAME = "test_name"

    def __init__(self, test_env: TestEnv) -> None:
        super().__init__(test_env)
        self.__test_root = Path(self.config.test_suite.test_root)
        self.precompiled_test_files: bool = cast(bool, self.config.test_suite.get_parameter("precompiled-test-files"))
        self.workflow_kind = self.config.workflow.workflow_type

    @property
    def gtest_root(self) -> Path:
        return self.__test_root

    @staticmethod
    def get_class_and_tests(test_file: str, qemu_prefix: list, file_path: Path | None = None,
                            raw_files_from_dir: bool = True) -> dict[str, list[str]]:
        # test_file name may be:
        # - test_file
        # - test_file/test_class - in this case we need additional filter arg to get test names
        cmd: list[str | Path]
        if not raw_files_from_dir and file_path:
            test_file_name, _, class_name = test_file.partition("/")
            cmd = [(file_path / test_file_name).as_posix(), "--gtest_list_tests"]
            if class_name:
                cmd.append(f"--gtest_filter={class_name}.*")
        else:
            cmd = [test_file, "--gtest_list_tests"]

        if qemu_prefix:
            cmd = [*qemu_prefix, *cmd]

        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False
        )

        class_test_names = GTestSuite.parse_test_string_regex(result.stdout)
        return class_test_names

    @staticmethod
    def parse_test_string_regex(gtest_list_tests: str) -> dict[str, list[str]]:
        pattern = r'([A-Za-z0-9_]+)\.\s*\n((?:(?!^[A-Za-z]+\.).)*)'
        matches = re.findall(pattern, gtest_list_tests, re.MULTILINE | re.DOTALL)

        result = {}
        for test_class, test_names_str in matches:
            test_names = [m.strip() for m in test_names_str.split('\n') if m.strip()]
            result[test_class] = test_names

        return result

    @staticmethod
    def _load_gtests(test_lists: list[Path]) -> list[str]:
        tests = []
        for test_list in test_lists:
            with open(test_list, encoding="utf-8") as t_file:
                lines = t_file.readlines()
                tests.extend([test.rstrip() for test in lines if test.rstrip('\n') and not test.startswith("#")])

        return tests

    @staticmethod
    def _define_file_kind(test_name: str) -> tuple[str, str]:
        """
        Possible explicit file names variants:
        - ani_test_any_call/AnyCallTest.AnyCall_Valid
        - ani_test_any_call/AnyCallTest
        - ani_test_any_call
        """
        test_file_re = re.compile(
            r'^(?P<file_name>[a-z0-9_]+)'  # ani_test_any_call
            r'(?:/(?P<cls_name>[A-Z][A-Za-z0-9]*))?'  # /AnyCallTest
            r'(?:\.(?P<test_name>[A-Za-z0-9_]+))?$'  # .AnyCall_Valid
        )
        name_match = test_file_re.fullmatch(test_name)
        if not name_match:
            raise InvalidGTestNameException(f"Invalid test name: {test_name}")
        file_name_groups = name_match.groupdict()
        if file_name_groups[GTestSuite.TEST_NAME]:
            kind = GTestSuite.TEST_NAME
        elif file_name_groups[GTestSuite.TEST_FILE] and file_name_groups[GTestSuite.TEST_CLASS_NAME]:
            kind = GTestSuite.TEST_CLASS_NAME
        else:
            kind = GTestSuite.TEST_FILE

        return kind, test_name

    @staticmethod
    def _load_line(line: str, test_root: Path) -> Path | None:
        test, _ = TestSuite._get_test_and_comment_from_line(line.strip(" \n"))
        if test is None:
            return None
        test_path = test_root / test
        return test_path

    @staticmethod
    def _load_list(test_root: Path, test_list_path: Path) -> list[Path]:
        result: list[Path] = []
        if not test_list_path.exists():
            return result

        with os.fdopen(os.open(test_list_path, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_handler:
            for line in file_handler:
                test_path = GTestSuite._load_line(line, test_root)
                if test_path is not None:
                    result.append(test_path)
        return result

    def _process_tests(self, raw_set: list[Path], force_generate: bool, name: str, update_explicit: bool) -> None:
        kind, explicit_name = GTestSuite._define_file_kind(name)
        match kind:
            case GTestSuite.TEST_FILE | GTestSuite.TEST_CLASS_NAME:
                raw_set.extend(self._get_raw_set(force_generate, explicit_name))
                if update_explicit:
                    self._explicit_file = None
            case GTestSuite.TEST_NAME:
                path_to_test = correct_path(self.gtest_root, explicit_name)
                if update_explicit:
                    self._explicit_file = path_to_test
                else:
                    raw_set.append(path_to_test)

    def process(self, force_generate: bool) -> list[ITestFlow]:
        raw_set: list[Path] = []
        self._explicit_file = self._set_explicit_file()
        self._explicit_list = self._set_explicit_list()
        if self._explicit_file is not None:
            self._process_tests(raw_set, force_generate, str(self._explicit_file), update_explicit=True)
        elif self.explicit_list is not None:
            tests_from_lists = GTestSuite._load_gtests([self.explicit_list])
            for test_name in tests_from_lists:
                self._process_tests(raw_set, force_generate, test_name, update_explicit=False)
        else:
            raw_set = self._get_raw_set(force_generate)

        executed_set = list(set(self._get_executed_files(raw_set)))
        self._search_both_excluded_and_ignored_tests()
        executed_set = self._get_by_groups(executed_set)
        tests = self._create_tests(executed_set)
        Log.default(_LOGGER, f"Loaded {len(tests)} valid tests from the folder '{self.gtest_root}'. "
                             f"Excluded {self.excluded} tests are not loaded.")
        return tests

    def _create_test(self, test_file: Path, is_ignored: bool) -> ITestFlow:

        test_id = get_test_id(test_file, self.gtest_root)

        coll_name = self._get_coll_name(test_id)
        params = self._collections_parameters.get(coll_name, {}) if coll_name is not None else {}

        test = workflow_registry.create(self.workflow_kind, test_env=self._test_env, test_path=test_file,
                                        params=IOptions(params), test_id=test_id)
        test.ignored = is_ignored
        if is_ignored and test.path in self.ignored_tests_with_failures:
            test.last_failure = self.ignored_tests_with_failures[test.path]

        return test

    def _get_raw_set(self, force_generate: bool, file_name: str | None = None) -> list[Path]:
        return self.__load_precompiled_files(file_name)

    def _get_test_root_pattern(self, mask: str) -> re.Pattern:
        return re.compile(f"{self.gtest_root / mask}")

    def _set_explicit_file(self) -> Path | None:
        test_id = self.config.test_suite.test_lists.explicit_file
        if test_id is not None and self.list_root is not None:
            return Path(test_id)
        return None

    def _get_executed_files(self, raw_test_files: list[Path]) -> list[Path]:
        """
        Browse the directory, search for files with the specified extension
        """
        _LOGGER.summary(f"Loading tests from the directory {self.test_root}")
        test_files: list[Path] = []
        if self.explicit_file is not None:
            test_files.append(self.explicit_file)
        else:
            if not self.config.test_suite.test_lists.skip_test_lists:
                self.excluded_tests = self._load_excluded_tests()
                self.ignored_tests = self._load_ignored_tests()
                self.ignored_tests_with_failures = self._get_tests_from_ignore_with_failures(
                    self._test_lists.ignored_lists)
            test_files.extend([test_file for test_file in raw_test_files if test_file not in self.excluded_tests])
        return test_files

    def _load_tests_from_lists(self, lists: list[Path]) -> list[Path]:
        tests = []

        for list_path in lists:
            _LOGGER.default(f"Loading tests from the list {list_path}")
            loaded = self._load_list(self.gtest_root, list_path)
            tests.extend(loaded)

        return tests

    def __load_precompiled_files(self, file_name: str | None) -> list[Path]:
        """
        Load precompiled test files from work dir
        """
        tests = []
        test_dir = self.gtest_root
        use_qemu: QemuKind = self.config.general.qemu
        if not use_qemu.value == 'none':
            cmd_prefix = self._test_env.cmd_prefix
        else:
            cmd_prefix = []

        if file_name is None:
            # file name is not set, get all files from tests dir, then get all tests from these files
            pattern = os.path.join(test_dir, '**', '*')
            tests_paths = fnmatch.filter(glob(pattern, recursive=True),
                                         path.join(self.gtest_root, self.config.test_suite.filter))
            for file_path in tests_paths:
                if os.path.isfile(file_path) and is_executable_file(Path(file_path)):
                    test_class_names = GTestSuite.get_class_and_tests(file_path, cmd_prefix)
                    for test_class, test_names in test_class_names.items():
                        tests.extend([f"{file_path}/{test_class}.{test_name}" for test_name in test_names])
            return [Path(test) for test in set(tests)]

        # file name is set -> get all tests from this file
        test_class_names = GTestSuite.get_class_and_tests(file_name, cmd_prefix, test_dir, raw_files_from_dir=False)
        for test_class, test_names in test_class_names.items():
            if test_class in file_name:
                path_to_test = f"{test_dir}/{file_name}"
            else:
                path_to_test = f"{test_dir}/{file_name}/{test_class}"
            tests.extend([f"{path_to_test}.{test_name}" for test_name in test_names])
        return [Path(test) for test in set(tests)]

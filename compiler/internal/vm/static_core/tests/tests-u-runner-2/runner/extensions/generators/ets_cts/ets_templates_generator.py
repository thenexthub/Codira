#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
import glob
import os
import shutil
from functools import cached_property
from pathlib import Path

from runner.extensions.generators.ets_cts.benchmark import Benchmark
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.suites.test_metadata import TestMetadata

_LOGGER = Log.get_logger(__file__)

EXPECTED_FILE_EXTENSION = ".expected"


class EtsTemplatesGenerator(IGenerator):
    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self.extension = f".{self._config.test_suite.extension()}"
        self.filter: str = str(self._config.test_suite.get_parameter("filter"))
        self.test_file = self._config.test_suite.get_parameter("test-file")
        self._already_generated: set[Path] = set()

    @staticmethod
    def get_expected_files_for_test(test_path: Path) -> list[Path]:
        test_parent_dir = test_path.parent
        test_expected = []
        for file_name in test_parent_dir.iterdir():
            if test_path.stem in str(file_name) and EXPECTED_FILE_EXTENSION in str(file_name):
                test_expected.append(file_name)
        return test_expected

    @cached_property
    def _matched(self) -> set[Path]:
        return self._get_matched_paths()

    def generate(self) -> list[str]:
        _LOGGER.all("Starting generate test")
        if self._target.exists():
            shutil.rmtree(self._target)
        test_source = self._source
        if self.test_file and fnmatch.fnmatchcase(self.test_file, self.filter):
            test_source = self._source / self.test_file
        self.__dfs(test_source, self._already_generated)

        _LOGGER.all("Generation finished!")
        return self.generated_tests

    def _get_matched_paths(self) -> set[Path]:
        matched: set[Path] = set()
        test_root = Path(self._source)
        flt = self.filter

        patterns = [flt, f"{flt}/**/*"]

        for pattern in patterns:
            for rel in glob.iglob(pattern, root_dir=str(test_root), recursive=True):
                p = test_root / rel
                if p.is_file() and str(p).endswith(self.extension):
                    matched.add(p)
        return matched

    def _copy_expected_file(self, path: Path) -> None:
        test_full_name = path.relative_to(self._source)
        output = self._target / test_full_name
        shutil.copy(path, output.parent)

    def _strip_test_template_suffix(self, path: Path) -> Path | None:
        if path.exists():
            return path

        if not self.test_file:
            return None

        # test_file might have postfix from template, should find parent file
        base, _, _ = path.name.rpartition("_")
        parent_test = path.parent / f"{base}{self.extension}"
        return parent_test if parent_test.exists() else None

    def __generate_test(self, path: Path) -> None:
        test_full_name = os.path.relpath(path, self._source)
        output = self._target / test_full_name
        bench = Benchmark(path, output, test_full_name, self._source, self.extension)
        generated_tests = bench.generate()
        dep_tests_to_generate: set[Path] = set()
        for test in generated_tests:
            test_path = self._source / test_full_name
            test_metadata = TestMetadata.get_metadata(Path(test))
            if test_metadata.files:
                dep_tests_to_generate |= {Path(test) for test in test_metadata.files}

            test_parent_dir = test_path.parent
            for file_name in test_parent_dir.iterdir():
                if file_name.is_file() and file_name.suffixes[-2:] == [".d", ".ets"]:
                    dep_tests_to_generate.add(Path(file_name.name))

        self.generated_tests.extend(generated_tests)
        dep_tests_to_generate = {(path.parent / Path(dep_test)).resolve() for dep_test in dep_tests_to_generate}
        if dep_tests_to_generate:
            for dep_test in dep_tests_to_generate:
                dep_test_output = self._target / dep_test.relative_to(self._source)
                if not dep_test_output.exists():
                    self.__generate_test(dep_test)
                    self._already_generated.add(dep_test)

    def __dfs(self, test_path: Path, seen: set[Path]) -> None:
        path = self._strip_test_template_suffix(test_path)
        if path is None or path in seen:
            return

        seen.add(path)

        if path.is_dir():
            for child in sorted(path.iterdir()):
                self.__dfs(child, seen)
            return

        if path not in self._matched:
            return

        if path.suffix == self.extension:
            self.__generate_test(path)

            test_expected_files = EtsTemplatesGenerator.get_expected_files_for_test(path)
            if test_expected_files:
                for expected_file in test_expected_files:
                    self._copy_expected_file(expected_file)

            return

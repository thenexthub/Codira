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

import argparse
from functools import cached_property
from pathlib import Path
from typing import Any, cast

from runner.options.options import IOptions
from runner.utils import make_dir_if_not_exist

USE_LLVM_COV = "use-llvm-cov"
USE_LCOV = "use-lcov"
PROFDATA_FILES_DIR = "profdata-files-dir"
COVERAGE_HTML_REPORT_DIR = "coverage-html-report-dir"
COVERAGE_PER_BINARY = "coverage-per-binary"
LLVM_COV_EXCLUDE = "llvm-cov-exclude"
LCOV_EXCLUDE = "lcov-exclude"
CLEAN_GCDA_BEFORE_RUN = "clean-gcda-before-run"
GCOV_TOOL = "gcov-tool"


class CoverageOptions(IOptions):
    def __init__(self, args: dict[str, Any]):  # type: ignore[explicit-any]
        super().__init__(args)
        self._parameters = args

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        """
        Add coverage-related command-line arguments to an argument parser.

        Args:
            parser (argparse.ArgumentParser): The parser to add arguments to.
            dest (str | None): Optional prefix for argument destinations.
        """
        parser.add_argument(
            f'--{USE_LLVM_COV}', action='store_true', default=False,
            dest=f"{dest}{USE_LLVM_COV}")
        parser.add_argument(
            f'--{USE_LCOV}', action='store_true', default=False,
            dest=f"{dest}{USE_LCOV}")
        parser.add_argument(
            f'--{COVERAGE_PER_BINARY}', action='store_true', default=False,
            dest=f"{dest}{COVERAGE_PER_BINARY}")
        parser.add_argument(
            f'--{PROFDATA_FILES_DIR}', default=None,
            type=make_dir_if_not_exist,
            dest=f"{dest}{PROFDATA_FILES_DIR}",
            help='Directory where coverage intermediate files (*.profdata) are created.')
        parser.add_argument(
            f'--{COVERAGE_HTML_REPORT_DIR}', default=None,
            type=make_dir_if_not_exist,
            dest=f"{dest}{COVERAGE_HTML_REPORT_DIR}",
            help='Stacks files in the specified directory')
        parser.add_argument(
            f'--{LLVM_COV_EXCLUDE}',
            action='append',
            dest=f"{dest}{LLVM_COV_EXCLUDE}",
            help="Add one or more REGEX exclude patterns for llvm-cov")
        parser.add_argument(
            f'--{LCOV_EXCLUDE}',
            action='append',
            dest=f"{dest}{LCOV_EXCLUDE}",
            help="Add one or more GLOB exclude patterns for lcov")
        parser.add_argument(
            f'--{CLEAN_GCDA_BEFORE_RUN}', action='store_true', default=False,
            dest=f"{dest}{CLEAN_GCDA_BEFORE_RUN}")
        parser.add_argument(
            f'--{GCOV_TOOL}', default=None,
            type=str,
            dest=f"{dest}{GCOV_TOOL}",
            help='Specify gcov binary name (Needed for lcov --gcov-tool)')

    @cached_property
    def use_llvm_cov(self) -> bool:
        """
        Whether to use LLVM's coverage tool (`llvm-cov`).

        Returns:
            bool: True if `--use-llvm-cov` was passed; False otherwise.
        """
        return cast(bool, self._parameters[USE_LLVM_COV])

    @cached_property
    def use_lcov(self) -> bool:
        """
        Whether to use LCOV as the coverage tool.

        Returns:
            bool: True if `--use-lcov` was passed; False otherwise.
        """
        return cast(bool, self._parameters[USE_LCOV])

    @cached_property
    def coverage_per_binary(self) -> bool:
        """
        Whether to generate coverage reports per binary.

        Returns:
            bool: True if `--coverage-per-binary` was passed; False otherwise.
        """
        return cast(bool, self._parameters[COVERAGE_PER_BINARY])

    @cached_property
    def profdata_files_dir(self) -> Path | None:
        """
        Directory where `.profdata` files will be saved.

        Returns:
            Path | None: Path to the directory, or None if not set.
        """
        value = self._parameters[PROFDATA_FILES_DIR]
        return Path(value) if value is not None else value

    @cached_property
    def coverage_html_report_dir(self) -> Path | None:
        """
        Directory where HTML coverage reports will be generated.

        Returns:
            Path | None: Path to the HTML report directory, or None if not set.
        """
        value = self._parameters[COVERAGE_HTML_REPORT_DIR]
        return Path(value) if value is not None else value

    @cached_property
    def llvm_cov_exclude(self) -> list[str] | None:
        """
        List of REGEX patterns of files/dirs to exclude from LLVM coverage.

        Returns:
            list[str] | None: List of REGEX exclude patterns, or None if not set.
        """
        return self._parameters.get(LLVM_COV_EXCLUDE)

    @cached_property
    def lcov_exclude(self) -> list[str] | None:
        """
        List of GLOB patterns of files/dirs to exclude from LCOV coverage.

        Returns:
            list[str] | None: List of GLOB exclude patterns, or None if not set.
        """
        return self._parameters.get(LCOV_EXCLUDE)

    @cached_property
    def clean_gcda_before_run(self) -> bool:
        """
        Whether to clean `.gcda` files before starting coverage collection.

        Returns:
            bool: True if `--clean-gcda-before-run` was passed; False otherwise.
        """
        return cast(bool, self._parameters[CLEAN_GCDA_BEFORE_RUN])

    @cached_property
    def gcov_tool(self) -> str | None:
        """Specify gcov binary name (Needed for lcov --gcov-tool).

        Returns:
            Optional[str]: gcov binary name if specified, None otherwise.
        """
        return self._parameters.get(GCOV_TOOL)

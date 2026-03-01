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

from functools import cached_property
from pathlib import Path

from runner.options.options_general import GeneralOptions


class CoverageDir:
    def __init__(self, general: GeneralOptions, work_dir: Path):
        self.__general = general
        self.__work_dir = work_dir

    @property
    def html_report_dir(self) -> Path:
        """
        Get or create the directory for HTML coverage reports.

        If a custom HTML report directory is specified in general options,
        it will be used. Otherwise, a default subdirectory under root is created.

        Returns:
            Path: Path to the HTML report directory.
        """
        if self.__general.coverage.coverage_html_report_dir:
            html_out_path = self.__general.coverage.coverage_html_report_dir
        else:
            html_out_path = self.root / "html_report_dir"
        html_out_path.mkdir(parents=True, exist_ok=True)
        return html_out_path

    @property
    def info_file(self) -> Path:
        """
        Get the path to the LCOV format coverage info file.

        Returns:
            Path: Path to the coverage_lcov_format.info file.
        """
        return self.coverage_work_dir / "coverage_lcov_format.info"

    @property
    def json_file(self) -> Path:
        """
        Get the path to the JSON format coverage file.

        Returns:
            Path: Path to the coverage_json_format.json file.
        """
        return self.coverage_work_dir / "coverage_json_format.json"

    @property
    def profdata_files_list_file(self) -> Path:
        """
        Get the path to the file listing all .profdata files.

        Returns:
            Path: Path to the profdatalist.txt file.
        """
        return self.coverage_work_dir / "profdatalist.txt"

    @property
    def profdata_merged_file(self) -> Path:
        """
        Get the path to the merged .profdata file.

        This file typically contains the combined profiling data from multiple
        test runs, used for generating aggregated coverage reports.

        Returns:
            Path: The full path to the merged.profdata file.
        """
        return self.coverage_work_dir / "merged.profdata"

    @cached_property
    def root(self) -> Path:
        """
        Get the root coverage directory path, creating it if necessary.

        This directory serves as the base directory for all coverage-related files
        and directories. It is created inside the working directory and named 'coverage'.

        Returns:
            Path: The full path to the root coverage directory.
        """
        root = self.__work_dir / "coverage"
        root.mkdir(parents=True, exist_ok=True)
        return root

    @cached_property
    def profdata_dir(self) -> Path:
        """
        Get the directory for storing .profdata files, creating it if necessary.

        If a custom profdata directory is specified in the general configuration,
        that will be used. Otherwise, a default subdirectory under the root directory
        named 'profdata_dir' is created.

        Returns:
            Path: The full path to the profdata directory.
        """
        if self.__general.coverage.profdata_files_dir:
            profdata_out_path = self.__general.coverage.profdata_files_dir
        else:
            profdata_out_path = self.root / "profdata_dir"
        profdata_out_path.mkdir(parents=True, exist_ok=True)
        return profdata_out_path

    @cached_property
    def coverage_work_dir(self) -> Path:
        """
        Get the working directory for coverage processing, creating it if necessary.

        This directory is used as a temporary workspace during coverage data processing.
        It is created inside the root coverage directory and named 'work_dir'.

        Returns:
            Path: The full path to the coverage working directory.
        """
        work_dir = self.root / "work_dir"
        work_dir.mkdir(parents=True, exist_ok=True)
        return work_dir

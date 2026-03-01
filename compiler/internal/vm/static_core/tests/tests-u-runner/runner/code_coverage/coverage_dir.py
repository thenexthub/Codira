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

from functools import cached_property
from pathlib import Path
from runner.options.options_general import GeneralOptions


class CoverageDir:
    def __init__(self, general: GeneralOptions, work_dir: Path):
        self.__general = general
        self.__work_dir = work_dir

    @property
    def html_report_dir(self) -> Path:
        if self.__general.coverage.coverage_html_report_dir:
            html_out_path = Path(self.__general.coverage.coverage_html_report_dir)
        else:
            html_out_path = self.root / "html_report_dir"
        html_out_path.mkdir(parents=True, exist_ok=True)
        return html_out_path

    @property
    def info_file(self) -> Path:
        return self.coverage_work_dir / "coverage_lcov_format.info"

    @property
    def json_file(self) -> Path:
        return self.coverage_work_dir / "coverage_json_format.json"

    @property
    def profdata_files_list_file(self) -> Path:
        return self.coverage_work_dir / "profdatalist.txt"

    @property
    def profdata_merged_file(self) -> Path:
        return self.coverage_work_dir / "merged.profdata"

    @cached_property
    def root(self) -> Path:
        root = self.__work_dir / "coverage"
        root.mkdir(parents=True, exist_ok=True)
        return root

    @cached_property
    def profdata_dir(self) -> Path:
        if self.__general.coverage.profdata_files_dir:
            profdata_out_path = Path(self.__general.coverage.profdata_files_dir)
        else:
            profdata_out_path = self.root / "profdata_dir"
        profdata_out_path.mkdir(parents=True, exist_ok=True)
        return profdata_out_path

    @cached_property
    def coverage_work_dir(self) -> Path:
        work_dir = self.root / "work_dir"
        work_dir.mkdir(parents=True, exist_ok=True)
        return work_dir

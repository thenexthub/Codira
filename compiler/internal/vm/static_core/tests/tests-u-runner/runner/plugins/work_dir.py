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
from runner.code_coverage.coverage_dir import CoverageDir


class WorkDir:
    def __init__(self, general: GeneralOptions, default_work_dir: Path):
        self.__general = general
        self.__default_work_dir = default_work_dir

    @property
    def report(self) -> Path:
        return self.root / "report"

    @property
    def gen(self) -> Path:
        return self.root / "gen"

    @property
    def intermediate(self) -> Path:
        return self.root / "intermediate"

    @cached_property
    def root(self) -> Path:
        root = Path(self.__general.work_dir) if self.__general.work_dir else self.__default_work_dir
        root.mkdir(parents=True, exist_ok=True)
        return root

    @cached_property
    def coverage_dir(self) -> CoverageDir:
        return CoverageDir(self.__general, self.root)

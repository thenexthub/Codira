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

from dataclasses import dataclass
from pathlib import Path

from runner.code_coverage.coverage_manager import CoverageManager
from runner.options.config import Config
from runner.reports.report_format import ReportFormat
from runner.suites.work_dir import WorkDir


@dataclass
class TestEnv:
    config: Config

    cmd_prefix: list[str]
    cmd_env: dict[str, str]

    timestamp: int
    report_formats: set[ReportFormat]
    work_dir: WorkDir

    coverage: CoverageManager


@dataclass(frozen=True)
class BinaryParams:
    timeout: int
    executor: Path
    flags: list[str]
    env: dict[str, str]
    step_filter: str = "*"
    use_qemu: bool = False

    @property
    def component_name(self) -> str:
        return self.executor.stem


@dataclass
class TestReport:
    output: str
    error: str
    return_code: int

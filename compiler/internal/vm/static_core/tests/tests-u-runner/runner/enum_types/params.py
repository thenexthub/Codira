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

from dataclasses import dataclass
from typing import Any, List, Set, Optional, Dict

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.options.config import Config
from runner.reports.report_format import ReportFormat
from runner.code_coverage.coverage_manager import CoverageManager
from runner.plugins.work_dir import WorkDir


@dataclass
class TestEnv:
    config: Config

    conf_kind: ConfigurationKind

    cmd_prefix: List[str]
    cmd_env: Dict[str, str]

    coverage: CoverageManager

    # Path and parameters for es2panda binary
    es2panda: str
    es2panda_args: List[str]

    # Path and parameters for ark binary
    runtime: str
    runtime_args: List[str]

    # Path and parameters for ark_aot binary
    ark_aot: Optional[str]
    aot_args: List[str]

    # Path and parameters for ark_quick binary
    ark_quick: str
    quick_args: List[str]

    timestamp: int
    report_formats: Set[ReportFormat]
    work_dir: WorkDir

    # Path and parameters for verifier binary
    verifier: str
    verifier_args: List[str]

    util: Any = None


@dataclass(frozen=True)
class Params:
    timeout: int
    gdb_timeout: int
    executor: str
    fail_kind_fail: FailKind
    fail_kind_timeout: FailKind
    fail_kind_other: FailKind
    flags: List[str]
    env: Dict[str, str]


@dataclass(frozen=True)
class TestReport:
    output: str
    error: str
    return_code: int

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import platform
import threading
from pathlib import Path
from typing import Optional, Tuple

from runner.code_coverage.cmd_executor import CmdExecutor, LinuxCmdExecutor
from runner.code_coverage.coverage_dir import CoverageDir
from runner.code_coverage.lcov_tool import LcovTool
from runner.code_coverage.llvm_cov_tool import LlvmCovTool
from runner.options.options_coverage import CoverageOptions


class CoverageManager:
    _instance: Optional['CoverageManager'] = None
    _lock: threading.Lock = threading.Lock()
    _initialized: bool = False

    def __new__(
        cls,
        build_dir_path: Optional[Path] = None,
        coverage_dir: Optional[CoverageDir] = None,
        coverage_options: Optional[CoverageOptions] = None
    ) -> 'CoverageManager':
        if cls._instance is not None:
            return cls._instance
        with cls._lock:
            build_dir_path, coverage_dir = cls._validate_parameters(build_dir_path, coverage_dir)
            gcov_tool = None if coverage_options is None else coverage_options.gcov_tool
            if cls._instance is None:
                instance = super().__new__(cls)
                instance._initialize(build_dir_path, coverage_dir, gcov_tool)
                cls._instance = instance
        return cls._instance

    @property
    def llvm_cov_tool(self) -> LlvmCovTool:
        return self.llvm_cov

    @property
    def lcov_tool(self) -> LcovTool:
        return self.lcov

    @staticmethod
    def _get_cmd_executor() -> CmdExecutor:
        system = platform.system()
        if system == "Linux":
            return LinuxCmdExecutor()
        raise NotImplementedError(f"Unsupported OS: {system}")

    @classmethod
    def _validate_parameters(
        cls,
        build_dir_path: Optional[Path] = None,
        coverage_dir: Optional[CoverageDir] = None
    ) -> Tuple[Path, CoverageDir]:
        if build_dir_path is None:
            raise ValueError("build_dir_path is not initialized")
        if not build_dir_path.parts or str(build_dir_path) == ".":
            raise ValueError("build_dir_path must not be '.' or empty")
        if not build_dir_path.exists():
            raise FileNotFoundError(f"Directory build_dir_path={build_dir_path} not found")

        if coverage_dir is None:
            raise ValueError("coverage_dir is not initialized")
        if not coverage_dir.root.exists():
            raise FileNotFoundError(f"Directory coverage_dir.root={coverage_dir.root} not found")

        return build_dir_path, coverage_dir

    def _initialize(self, build_dir_path: Path, coverage_dir: CoverageDir, gcov_tool: Optional[str]) -> None:
        if self._initialized:
            return
        self.cmd_executor = self._get_cmd_executor()
        self.llvm_cov = LlvmCovTool(build_dir_path, coverage_dir, self.cmd_executor)
        self.lcov = LcovTool(build_dir_path, coverage_dir, self.cmd_executor, gcov_tool)
        self._initialized = True

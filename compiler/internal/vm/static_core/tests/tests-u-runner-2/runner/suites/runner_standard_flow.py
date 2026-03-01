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
import shutil
from datetime import datetime
from pathlib import Path
from typing import cast

import pytz

from runner import utils
from runner.code_coverage.coverage_manager import CoverageManager
from runner.common_exceptions import InvalidConfiguration
from runner.cpumask import CPUMask
from runner.enum_types.params import TestEnv
from runner.extensions.suites.test_suite_registry import suite_registry
from runner.logger import Log
from runner.options.config import Config
from runner.options.step import StepKind
from runner.runner_file_based import RunnerFileBased
from runner.suites.work_dir import WorkDir
from runner.test_base import Test

_LOGGER = Log.get_logger(__file__)


class RunnerStandardFlow(RunnerFileBased):
    __DEFAULT_SUITE_NAME = "custom-scheme"

    def __init__(self, config: Config):
        self.suite_name = config.test_suite.suite_name if config.test_suite.suite_name else self.__DEFAULT_SUITE_NAME
        self.steps = config.workflow.steps
        RunnerFileBased.__init__(self, config, self.suite_name)

        self.__aot_check()

        self.test_env: TestEnv = TestEnv(
            config=config,
            cmd_prefix=self.cmd_prefix,
            cmd_env=self.cmd_env,
            timestamp=int(datetime.timestamp(datetime.now(pytz.UTC))),
            report_formats={self.config.general.report_format},
            work_dir=WorkDir(config, self.default_work_dir_root),
            coverage=CoverageManager(self.config.general.build, self.work_dir.coverage_dir, config.general.coverage),
        )

        self.__remove_intermediate_files()
        self.test_suite_kind = self.config.workflow.workflow_type

        test_suite = suite_registry.create(self.test_suite_kind, self.test_env)

        self.tests = set(cast(list[Test], test_suite.process(self.config.test_suite.ets.force_generate)))
        self.excluded = test_suite.excluded
        self.excluded_tests.update(test_suite.excluded_tests)
        self.list_root = test_suite.list_root
        self.test_root = self.test_env.work_dir.gen

        cpu_mask = self.config.test_suite.get_parameter("mask")
        self.cpu_mask: CPUMask | None = CPUMask(cpu_mask) \
            if cpu_mask is not None and isinstance(cpu_mask, str) else None

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / self.suite_name

    def create_execution_plan(self) -> None:
        _LOGGER.short(f"\n{utils.pretty_divider()}")
        _LOGGER.short(f"Execution plan for the test suite '{self.name}'")
        _LOGGER.short(self.config.workflow.pretty_str())

    def create_coverage_html(self) -> None:
        """
        Generate HTML coverage reports based on the selected coverage tool and configuration.

        This method determines which coverage tool to use (LLVM or LCOV), and whether to generate
        coverage per binary or as a single report. It orchestrates the creation of intermediate
        coverage files, merging them, exporting to `.info` format, and finally generating the HTML report.

        Uses:
            - `use_llvm_cov`: If True, uses LLVM-based tools (`profdata`, `llvm-cov`) for coverage.
            - `use_lcov`: If True, uses LCOV-based tools (`lcov`, `genhtml`) for coverage.
            - `coverage_per_binary`: If True, generates separate reports per component/binary.

        Steps for LLVM:
            1. Create list of `.profdata` files.
            2. Merge `.profdata` files.
            3. Export to `.info` file using `llvm-cov`.
            4. Generate HTML report using `genhtml`.

        Steps for LCOV:
            1. Generate `.info` file(s) using `lcov`.
            2. Generate HTML report using `genhtml`.

        Returns:
            None
        """
        _LOGGER.default("Create html report for coverage")
        coverage_configs = self.test_env.config.general.coverage
        llvm_tool = self.test_env.coverage.llvm_cov_tool
        lcov_tool = self.test_env.coverage.lcov_tool

        if coverage_configs.use_llvm_cov:
            if coverage_configs.coverage_per_binary:
                llvm_tool.make_profdata_list_files_by_components()
                llvm_tool.merge_all_profdata_files_by_components()
                llvm_tool.export_to_info_file_by_components(coverage_configs.llvm_cov_exclude)
            else:
                llvm_tool.make_profdata_list_file()
                llvm_tool.merge_all_profdata_files()
                llvm_tool.export_to_info_file(exclude_regex=coverage_configs.llvm_cov_exclude)
            llvm_tool.generate_html_report()
        elif coverage_configs.use_lcov:
            if coverage_configs.coverage_per_binary:
                lcov_tool.generate_dot_info_file_by_components(coverage_configs.lcov_exclude)
                lcov_tool.generate_html_report_by_components()
            else:
                lcov_tool.generate_dot_info_file(coverage_configs.lcov_exclude)
                lcov_tool.generate_html_report()

    def before_suite(self) -> None:
        if self.cpu_mask:
            self.cpu_mask.apply()

    def after_suite(self) -> None:
        if self.cpu_mask:
            self.cpu_mask.restore()

    def __aot_check(self) -> None:
        if self.config.general.aot_check:
            aot_check_passed: bool = False
            for step in self.steps:
                if step.executable_path is None:
                    continue
                all_args = " ".join(step.args)
                if step.executable_path.name == 'ark_js_vm' or (
                        step.executable_path.name == 'hdc' and all_args.find('ark_js_vm') > -1
                ):
                    aot_check_passed = aot_check_passed or all_args.find('--aot-file') > -1
            if not aot_check_passed:
                raise InvalidConfiguration(
                    f"AOT check FAILED. Check command line options of steps running `ark_js_vm` in workflow"
                    f"configuration file {self.config.workflow.name}. The expected option `--aot-file` absent."
                )
            _LOGGER.default("AOT check PASSED")

    def __remove_intermediate_files(self) -> None:
        compiler_step = [step for step in self.steps if step.step_kind == StepKind.COMPILER and step.enabled]
        if compiler_step:
            shutil.rmtree(self.test_env.work_dir.intermediate, ignore_errors=True)
            _LOGGER.default(f"Intermediate folder {self.test_env.work_dir.intermediate} has been removed. "
                            f"Check whether it exists: {self.test_env.work_dir.intermediate.exists()}")
        self.test_env.work_dir.intermediate.mkdir(parents=True, exist_ok=True)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2025 Huawei Device Co., Ltd.
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

import shutil
import uuid
from multiprocessing import current_process
from pathlib import Path

from runner.code_coverage.cmd_executor import CmdExecutor
from runner.code_coverage.coverage_dir import CoverageDir

IGNORE_COMPONENTS = ['npx']
LCOV_IGNORE_ERROR = ['empty', 'utility', 'range', 'inconsistent',
                     'source', 'format', 'category', 'mismatch',
                     'unused', 'path', 'missing']


class LcovTool:
    def __init__(
        self,
        build_dir_path: Path,
        coverage_dir: CoverageDir,
        cmd_executor: CmdExecutor,
        gcov_tool: str | None
    ) -> None:
        self.build_dir = build_dir_path
        self.bin_dir = self.build_dir / 'bin'
        self.coverage_dir = coverage_dir
        self.cmd_executor = cmd_executor
        self.gcov_tool = gcov_tool

        self.dot_info_files_dir = self.coverage_dir.coverage_work_dir / "dot_info_files_dir"

        self.lcov_binary = self.cmd_executor.get_binary("lcov")
        self.genhtml_binary = self.cmd_executor.get_binary("genhtml")

        self.ignore_errors = ','.join(LCOV_IGNORE_ERROR)

        self.components: dict[str, Path] = {}

    def generate_dot_info_file_by_components(self, exclude_regex: list[str] | None = None) -> None:
        """
        Generate `.info` files for each component using `generate_dot_info_file`.

        Iterates over all components and calls `generate_dot_info_file` to generate
        individual coverage info files per component.

        Args:
            exclude_regex (list[str] | None): Optional regex pattern to exclude certain files/dirs.
        """
        for key in IGNORE_COMPONENTS:
            self.components.pop(key, None)
        for key in self.components:
            self.generate_dot_info_file(exclude_regex, key)

    def generate_dot_info_file(
        self,
        exclude_regex: list[str] | None = None,
        component_name: str | None = None
    ) -> None:
        """
        Generate an LCOV `.info` file for a specific component or globally.

        If a component name is provided, it creates a dedicated directory for that component
        and generates a uniquely named `.info` file inside it. Otherwise, a global file is used.

        Args:
            exclude_regex (list[str] | None): Regex pattern(s) to filter out unwanted files from coverage.
            component_name (str | None): Name of the component to generate coverage for.
                If None, coverage is generated globally.

        Returns:
            None
        """
        dot_info_file = self.coverage_dir.info_file
        base_directory_option = self.build_dir
        directory_option = self.build_dir

        if component_name is not None:
            component_dir_path = self.dot_info_files_dir.joinpath(component_name)
            component_dir_path.mkdir(parents=True, exist_ok=True)

            directory_option = self.create_gcno_symlinks(component_dir_path, base_directory_option, component_name)

            dot_info_file = component_dir_path / f"{component_name}-{current_process().pid}-{uuid.uuid4()}.info"
            self.components[component_name] = dot_info_file

        lcov_command = [
            self.lcov_binary,
            "--capture",
            "--all",
            "--quiet",
            "--base-directory", str(base_directory_option),
            "--directory", str(directory_option),
            "--output-file", str(dot_info_file),
            "--branch-coverage",
            "--ignore-errors", self.ignore_errors
        ]
        if exclude_regex is not None:
            for pattern in exclude_regex:
                lcov_command.extend(["--exclude", pattern])

        if self.gcov_tool is not None:
            lcov_command.extend(["--gcov-tool", self.gcov_tool])

        self.cmd_executor.run_command(lcov_command)

    def generate_html_report_by_components(self) -> None:
        """
        Generate HTML coverage reports for each component individually.

        Uses the `generate_html_report` method to create separate HTML reports
        for each component's `.info` file.
        """
        for component_name, dot_info_file in self.components.items():
            html_report_dir = self.coverage_dir.html_report_dir / component_name
            self.generate_html_report(html_report_dir, dot_info_file)

    def generate_html_report(
        self,
        output_directory: Path | None = None,
        dot_info_file: Path | None = None
    ) -> None:
        """
        Generate an HTML coverage report from an `.info` file.

        Args:
            output_directory (Path | None): Directory where the HTML report will be saved.
                Defaults to `coverage_dir.html_report_dir`.
            dot_info_file (Path | None): The `.info` file to use as input for the report.
                Defaults to `coverage_dir.info_file`.

        Returns:
            None
        """
        _output_directory = output_directory or self.coverage_dir.html_report_dir
        _dot_info_file = dot_info_file or self.coverage_dir.info_file
        genhtml_command = [
            str(self.genhtml_binary),
            f"--output-directory={_output_directory}",
            str(_dot_info_file),
            "--ignore-errors", self.ignore_errors,
            "--filter", "missing"
        ]
        self.cmd_executor.run_command(genhtml_command)

    def clear_gcda_files(self) -> None:
        """
        Clear all `.gcda` coverage counter files by resetting them to zero.

        This uses the `lcov --zerocounters` command to reset counters in the build directory.

        Returns:
            None
        """
        lcov_command = [self.lcov_binary, '--zerocounters', '--directory', str(self.build_dir)]
        self.cmd_executor.run_command(lcov_command)

    def get_gcov_prefix(
        self,
        component_name: str,
    ) -> list[str]:
        """
        Get the gcov prefix options for a given component.

        Used when generating coverage data to control how paths are displayed
        in the final report.

        Args:
            component_name (str): Name of the component to set up prefix for.

        Returns:
            list[str]: A list containing the prefix path and strip level.
        """
        self.components[component_name] = Path.cwd()
        gcov_prefix = str(self.dot_info_files_dir.joinpath(component_name))
        gcov_prefix_strip = str(len(self.build_dir.parts) - 1)
        return [gcov_prefix, gcov_prefix_strip]

    def create_gcno_symlinks(self, gcda_root: Path, gcno_root: Path, component_name: str) -> Path:
        """
        Create symbolic links for `.gcno` and `.gcda` files in a temporary directory.

        This helps in separating coverage data for different components while maintaining
        correct relative paths expected by coverage tools.

        Args:
            gcda_root (Path): Root directory containing `.gcda` files.
            gcno_root (Path): Root directory containing `.gcno` files.
            component_name (str): Component name to scope symlink creation under.

        Returns:
            Path: Path to the newly created symlink directory.
        """
        symlink_dir = Path(f"/tmp/lcov_fake_build/{component_name}")

        shutil.rmtree(symlink_dir, ignore_errors=True)
        symlink_dir.mkdir(exist_ok=True, parents=True)

        for gcno in gcno_root.rglob("*.gcno"):
            rel_path = gcno.relative_to(gcno_root)
            target = symlink_dir / rel_path
            target.parent.mkdir(exist_ok=True, parents=True)
            target.symlink_to(gcno)

        for gcda in gcda_root.rglob("*.gcda"):
            rel_path = gcda.relative_to(gcda_root)
            target = symlink_dir / rel_path
            target.parent.mkdir(exist_ok=True, parents=True)
            target.symlink_to(gcda)

        return symlink_dir

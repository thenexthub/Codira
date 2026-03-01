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

import os
import uuid
from multiprocessing import current_process
from pathlib import Path

from runner.code_coverage.cmd_executor import CmdExecutor
from runner.code_coverage.coverage_dir import CoverageDir
from runner.utils import get_opener, write_2_file

LLVM_COV_TOOLS_VERSION_NUMBER = '14'


class LlvmCovTool:
    def __init__(self, build_dir_path: Path, coverage_dir: CoverageDir, cmd_executor: CmdExecutor) -> None:
        self.build_dir = build_dir_path
        self.build_bin_dir = self.build_dir / "bin"
        self.coverage_dir = coverage_dir
        self.cmd_executor = cmd_executor

        self.llvm_profdata_version = os.getenv("LLVM_PROFDATA_VERSION", LLVM_COV_TOOLS_VERSION_NUMBER)
        self.llvm_cov_version = os.getenv("LLVM_COV_VERSION", LLVM_COV_TOOLS_VERSION_NUMBER)

        self.llvm_profdata_binary = self.cmd_executor.get_binary("llvm-profdata", self.llvm_profdata_version)
        self.llvm_cov_binary = self.cmd_executor.get_binary("llvm-cov", self.llvm_cov_version)
        self.genhtml_binary = self.cmd_executor.get_binary("genhtml")

        self.components: dict[str, Path] = {}

    def get_uniq_profraw_profdata_file_paths(self, component_name: str | None = None) -> list[Path]:
        """
        Generate unique file paths for `.profraw` and `.profdata` files based on PID and UUID.

        If a component name is provided, the files are placed under a subdirectory named after the component.

        Args:
            component_name (str | None): Optional component name used to scope file paths.

        Returns:
            list[Path]: Paths to the generated `.profraw` and `.profdata` files.
        """
        file_name = f"{current_process().pid}-{uuid.uuid4()}"
        file_path = self.coverage_dir.profdata_dir / file_name

        if component_name is not None:
            self.components[component_name] = Path.cwd()
            component_profdata_dir = self.coverage_dir.profdata_dir / component_name
            component_profdata_dir.mkdir(parents=True, exist_ok=True)
            file_path = component_profdata_dir / f"{component_name}-{file_name}"

        profraw_file = file_path.with_suffix(".profraw")
        profdata_file = file_path.with_suffix(".profdata")
        return [profraw_file, profdata_file]

    def merge_and_delete_profraw_files(self, profraw_file: Path, profdata_file: Path) -> None:
        """
        Merge `.profraw` data into a `.profdata` file and delete the original `.profraw`.

        Args:
            profraw_file (Path): Path to the `.profraw` file containing raw profiling data.
            profdata_file (Path): Path where merged `.profdata` should be saved.

        Returns:
            None
        """
        if profraw_file.is_file():
            self._execute_profdata_merge(["--sparse", str(profraw_file), "-o", str(profdata_file)])
            os.remove(profraw_file)

    def make_profdata_list_file(
        self,
        profdata_root_dir_path: Path | None = None,
        profdata_files_list_file_path: Path | None = None
    ) -> None:
        """
        Create a file listing all `.profdata` files found in a directory.

        Args:
            profdata_root_dir_path (Path | None): Directory to search for `.profdata` files.
                Defaults to coverage_dir.profdata_dir.
            profdata_files_list_file_path (Path | None): Output file path for the list.
                Defaults to coverage_dir.profdata_files_list_file.

        Returns:
            None
        """
        _profdata_files_list_file_path = profdata_files_list_file_path or self.coverage_dir.profdata_files_list_file
        _profdata_root_dir_path = profdata_root_dir_path or self.coverage_dir.profdata_dir

        profdata_files = list(_profdata_root_dir_path.rglob("*.profdata"))

        write_2_file(_profdata_files_list_file_path, profdata_files)

    def make_profdata_list_files_by_components(self) -> None:
        """
        Generate individual `.profdatalist.txt` files for each component.

        Each component's list contains paths to its own `.profdata` files.

        Returns:
            None
        """
        for key in self.components:
            component_profdata_dir = self.coverage_dir.profdata_dir / key
            profdata_files_list_file_path = self._get_path_from_coverage_work_dir(f"{key}_profdatalist.txt")
            self.components[key] = profdata_files_list_file_path
            self.make_profdata_list_file(component_profdata_dir, profdata_files_list_file_path)

    def merge_all_profdata_files(
        self,
        profdata_files_list_file_path: Path | None = None,
        merged_profdata_file_path: Path | None = None
    ) -> None:
        """
        Merge multiple `.profdata` files into a single file.

        Args:
            profdata_files_list_file_path (Path | None): File containing list of `.profdata` files.
                Defaults to coverage_dir.profdata_files_list_file.
            merged_profdata_file_path (Path | None): Destination for merged output.
                Defaults to coverage_dir.profdata_merged_file.

        Returns:
            None
        """
        _merged_profdata_file_path = merged_profdata_file_path or self.coverage_dir.profdata_merged_file
        _profdata_files_list_file_path = profdata_files_list_file_path or self.coverage_dir.profdata_files_list_file

        args = [
            "--sparse",
            f"--input-files={_profdata_files_list_file_path}",
            "-o", str(_merged_profdata_file_path)]

        self._execute_profdata_merge(args)

    def merge_all_profdata_files_by_components(self) -> None:
        """
        Merge `.profdata` files per component into a single file for each.

        Returns:
            None
        """
        for component_name, profdata_files_list_file_path in self.components.items():
            merged_profdata_file_path = self._get_path_from_coverage_work_dir(f"{component_name}_merged.profdata")

            self.components[component_name] = merged_profdata_file_path

            self.merge_all_profdata_files(profdata_files_list_file_path, merged_profdata_file_path)

    def export_to_info_file(
        self,
        merged_profdata_file_path: Path | None = None,
        dot_info_file_path: Path | None = None,
        component_name: str | None = None,
        exclude_regex: list[str] | None = None
    ) -> None:
        """
        Export coverage data from `.profdata` to LCOV `.info` format.

        Args:
            merged_profdata_file_path (Path | None): Input `.profdata` file.
                Defaults to coverage_dir.profdata_merged_file.
            dot_info_file_path (Path | None): Output `.info` file.
                Defaults to coverage_dir.info_file.
            component_name (str | None): Optional component name for filtering.
            exclude_regex (list[str] | None): Regex pattern to exclude certain files from coverage.

        Returns:
            None
        """
        _merged_profdata_file_path = merged_profdata_file_path or self.coverage_dir.profdata_merged_file
        _dot_info_file_path = dot_info_file_path or self.coverage_dir.info_file

        command = [str(self.llvm_cov_binary), "export"]
        llvm_cov_export_command_args = ["-format=lcov", "-Xdemangler", "c++filt"]

        llvm_cov_export_command_args.extend(["-instr-profile", str(_merged_profdata_file_path)])

        if component_name is None:
            for binary_target_for_coverage in self.build_bin_dir.iterdir():
                if binary_target_for_coverage.is_file() and binary_target_for_coverage.suffix == '':
                    llvm_cov_export_command_args.extend(["--object", str(binary_target_for_coverage)])
        else:
            binary_target_for_coverage = self.build_bin_dir / component_name
            llvm_cov_export_command_args.extend(["--object", str(binary_target_for_coverage)])

        for so_path in self.build_dir.rglob('*.so'):
            llvm_cov_export_command_args.extend(["--object", str(so_path)])

        if exclude_regex is not None:
            llvm_cov_export_command_args.extend(["--ignore-filename-regex", "|".join(exclude_regex)])

        command.extend(llvm_cov_export_command_args)

        custom_opener = get_opener(0o644)
        with open(_dot_info_file_path, mode='w+', encoding='utf-8', opener=custom_opener) as file:
            self.cmd_executor.run_command(command, stdout=file)

    def export_to_info_file_by_components(self, exclude_regex: list[str] | None = None) -> None:
        """
        Export coverage data for each component into separate `.info` files.

        Args:
            exclude_regex (list[str] | None): Regex pattern to filter out unwanted files.

        Returns:
            None
        """
        for component_name, merged_profdata_file_path in self.components.items():
            dot_info_file_path = self._get_path_from_coverage_work_dir(f"{component_name}_lcov_format.info")
            self.components[component_name] = dot_info_file_path
            self.export_to_info_file(merged_profdata_file_path, dot_info_file_path, component_name, exclude_regex)

    def generate_html_report(self) -> None:
        """
        Generate HTML coverage reports from `.info` files.

        If components are defined, generates one report per component.
        Otherwise, generates a global report.

        Returns:
            None
        """
        if len(self.components):
            for component_name, dot_info_file in self.components.items():
                html_report_dir = Path(self.coverage_dir.html_report_dir).joinpath(component_name)
                output_directory_option = f"--output-directory={html_report_dir}"
                self._execute_genhtml([output_directory_option, str(dot_info_file)])
        else:
            output_directory_option = f"--output-directory={self.coverage_dir.html_report_dir}"
            self._execute_genhtml([output_directory_option, str(self.coverage_dir.info_file)])

    def _execute_genhtml(self, args: list[str]) -> None:
        self.cmd_executor.run_command([str(self.genhtml_binary), *args])

    def _execute_profdata_merge(self, args: list[str]) -> None:
        self.cmd_executor.run_command([str(self.llvm_profdata_binary), "merge", *args])

    def _get_path_from_coverage_work_dir(self, path_name: str) -> Path:
        return Path(self.coverage_dir.coverage_work_dir).joinpath(path_name)

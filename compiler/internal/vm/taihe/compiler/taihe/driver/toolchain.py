# coding=utf-8
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

import json
import logging
import os
import shutil
import subprocess
import tarfile
import time
from collections.abc import Iterable, Mapping, Sequence
from pathlib import Path

from taihe.driver.backend import BackendRegistry
from taihe.driver.contexts import CompilerInstance, CompilerInvocation
from taihe.utils.outputs import CMakeOutputConfig, OutputConfig
from taihe.utils.resources import (
    PandaVm,
    RuntimeHeader,
    RuntimeSource,
)

logger = logging.getLogger(__name__)


def run_command(
    command: Sequence[Path | str],
    capture_output: bool = False,
    env: Mapping[str, Path | str] | None = None,
) -> float:
    """Run a command with environment variables."""
    command_str = " ".join(map(str, command))

    env_str = ""
    for key, val in (env or {}).items():
        env_str += f"{key}={val} "

    logger.debug("+ %s%s", env_str, command_str)

    try:
        start_time = time.time()
        subprocess.run(
            command,
            check=True,
            text=True,
            env=env,
            capture_output=capture_output,
        )
        end_time = time.time()
        elapsed_time = end_time - start_time
        return elapsed_time
    except subprocess.CalledProcessError as e:
        logger.error("Command failed with exit code %s", e.returncode)
        if e.stdout:
            logger.error("Standard output: %s", e.stdout)
        if e.stderr:
            logger.error("Standard error: %s", e.stderr)
        raise


def create_directory(directory: Path) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    logger.debug("Created directory: %s", directory)


def clean_directory(directory: Path) -> None:
    if not directory.exists():
        return
    shutil.rmtree(directory)
    logger.debug("Cleaned directory: %s", directory)


def move_directory(src: Path, dst: Path) -> None:
    if not src.exists():
        raise FileNotFoundError(f"Source directory does not exist: {src}")
    shutil.move(src, dst)
    logger.debug("Moved directory from %s to %s", src, dst)


def copy_directory(src: Path, dst: Path) -> None:
    if not src.exists():
        raise FileNotFoundError(f"Source directory does not exist: {src}")
    shutil.copytree(src, dst, dirs_exist_ok=True)
    logger.debug("Copied directory from %s to %s", src, dst)


def extract_file(
    target_file: Path,
    extract_dir: Path,
) -> None:
    """Extract a tar.gz file."""
    if not target_file.exists():
        raise FileNotFoundError(f"File to extract does not exist: {target_file}")

    create_directory(extract_dir)

    with tarfile.open(target_file, "r:gz") as tar:
        # Check for any unsafe paths before extraction
        for member in tar.getmembers():
            member_path = Path(member.name)
            if member_path.is_absolute() or ".." in member_path.parts:
                raise ValueError(f"Unsafe path in archive: {member.name}")
        # Extract safely
        tar.extractall(path=extract_dir)

    logger.info("Extracted %s to %s", target_file, extract_dir)


def taihec(
    dst_dir: Path,
    src_files: list[Path],
    backend_names: list[str],
    buildsys_name: str | None = None,
    extra: dict[str, str | None] | None = None,
) -> None:
    registry = BackendRegistry()
    registry.register_all()
    backend_factories = registry.collect_required_backends(backend_names)
    backend_configs = [b() for b in backend_factories]

    if buildsys_name == "cmake":
        output_config = CMakeOutputConfig(
            dst_dir=dst_dir,
            runtime_include_dir=RuntimeHeader.resolve_path(),
            runtime_src_dir=RuntimeSource.resolve_path(),
        )
    else:
        output_config = OutputConfig(
            dst_dir=dst_dir,
        )

    invocation = CompilerInvocation(
        src_files=src_files,
        output_config=output_config,
        backend_configs=backend_configs,
        extra=extra or {},
    )
    instance = CompilerInstance(invocation)
    if not instance.run():
        raise RuntimeError("Taihe compiler (taihec) failed to run")


class CppToolchain:
    """Utility class for C++ toolchain operations."""

    def __init__(self):
        self.cxx = os.getenv("CXX", "clang++")
        self.cc = os.getenv("CC", "clang")

    def compile(
        self,
        output_dir: Path,
        input_files: Iterable[Path],
        include_dirs: Sequence[Path] = (),
        compile_flags: Sequence[str] = (),
    ) -> list[Path]:
        """Compile source files."""
        output_files: list[Path] = []

        for input_file in input_files:
            name = input_file.name
            output_file = output_dir / f"{name}.o"

            if name.endswith(".c"):
                compiler = self.cc
                std = "gnu11"
            else:
                compiler = self.cxx
                std = "gnu++17"

            command = [
                compiler,
                "-c",
                "-fvisibility=hidden",
                "-fPIC",
                # "-Wall",
                # "-Wextra",
                f"-std={std}",
                "-o",
                output_file,
                input_file,
                *compile_flags,
            ]

            for include_dir in include_dirs:
                if include_dir.exists():  # Only include directories that exist
                    command.append(f"-I{include_dir}")

            run_command(command)

            output_files.append(output_file)

        return output_files

    def link(
        self,
        output_file: Path,
        input_files: Sequence[Path],
        shared: bool = False,
        link_options: Sequence[str] = (),
    ) -> None:
        """Link object files."""
        if len(input_files) == 0:
            logger.warning("No input files to link")
            return

        command = [
            self.cxx,
            "-fPIC",
            "-o",
            output_file,
            *input_files,
            *link_options,
        ]

        if shared:
            command.append("-shared")

        run_command(command)

    def run(
        self,
        target: Path,
        ld_lib_path: Path,
        args: Sequence[str] = (),
    ) -> float:
        """Run the compiled target."""
        command = [
            target,
            *args,
        ]

        return run_command(
            command,
            env={"LD_LIBRARY_PATH": ld_lib_path},
        )


class ArkToolchain:
    """Utility class for ABC toolchain operations."""

    def __init__(self):
        self.vm = PandaVm.resolve()

    def create_arktsconfig(
        self,
        arktsconfig_file: Path,
        app_paths: dict[str, Path] | None = None,
    ) -> None:
        """Create ArkTS configuration file."""
        vm = PandaVm.resolve()
        paths = vm.stdlib_sources | vm.sdk_sources | (app_paths or {})

        config_content = {
            "compilerOptions": {
                "baseUrl": str(vm.host_tools_dir),
                "paths": {key: [str(value)] for key, value in paths.items()},
            }
        }

        with open(arktsconfig_file, "w") as json_file:
            json.dump(config_content, json_file, indent=2)

        logger.debug("Created configuration file at: %s", arktsconfig_file)

    def compile(
        self,
        output_dir: Path,
        input_files: Iterable[Path],
        arktsconfig_file: Path,
    ) -> list[Path]:
        """Compile ETS files to ABC format."""
        output_files: list[Path] = []

        for input_file in input_files:
            name = input_file.name
            output_file = output_dir / f"{name}.abc"
            output_dump = output_dir / f"{name}.abc.dump"

            gen_abc_command = [
                self.vm.tool("es2panda"),
                input_file,
                "--output",
                output_file,
                "--extension",
                "ets",
                "--arktsconfig",
                arktsconfig_file,
            ]

            run_command(gen_abc_command)

            output_files.append(output_file)

            ark_disasm_path = self.vm.tool("ark_disasm")
            if not ark_disasm_path.exists():
                logger.warning(
                    "ark_disasm not found at %s, skipping disassembly", ark_disasm_path
                )
                continue

            gen_abc_dump_command = [
                ark_disasm_path,
                output_file,
                output_dump,
            ]

            run_command(gen_abc_dump_command)

        return output_files

    def link(
        self,
        target: Path,
        input_files: Sequence[Path],
    ) -> None:
        """Link ABC files."""
        if len(input_files) == 0:
            logger.warning("No input files to link")
            return

        command = [
            self.vm.tool("ark_link"),
            "--output",
            target,
            "--",
            *input_files,
        ]

        run_command(command)

    def run(
        self,
        abc_target: Path,
        ld_lib_path: Path,
        entry: str,
    ) -> float:
        """Run the compiled ABC file with the Ark runtime."""
        ark_path = self.vm.tool("ark")

        command = [
            ark_path,
            f"--boot-panda-files={self.vm.sdk_lib}",
            f"--boot-panda-files={self.vm.stdlib_lib}",
            f"--load-runtimes=ets",
            abc_target,
            entry,
        ]

        return run_command(
            command,
            env={"LD_LIBRARY_PATH": ld_lib_path},
        )
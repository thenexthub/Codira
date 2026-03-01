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

import argparse
import logging
import sys
from abc import ABC, abstractmethod
from pathlib import Path

from taihe.driver.toolchain import (
    ArkToolchain,
    CppToolchain,
    clean_directory,
    create_directory,
    extract_file,
    move_directory,
    taihec,
)
from taihe.utils.logging import setup_logger
from taihe.utils.resources import (
    ResourceContext,
    RuntimeHeader,
    RuntimeSource,
    fetch_url,
)

logger = logging.getLogger(__name__)

# A lower value means more verbosity
TRACE_CONCISE = logging.DEBUG - 1
TRACE_VERBOSE = TRACE_CONCISE - 1


class BuildSystem(ABC):
    """Main build system class."""

    runtime_includes: list[Path]
    generated_includes: list[Path]
    author_includes: list[Path]

    runtime_sources: list[Path]

    author_backend_names: list[str]
    user_backend_names: list[str]

    def __init__(
        self,
        target_dir: str,
        verbosity: int = logging.INFO,
    ):
        self.should_run_pretty_print = verbosity <= logging.DEBUG

        self.cpp_toolchain = CppToolchain()

        # Build paths
        self.target_path = Path(target_dir).resolve()

        self.idl_dir = self.target_path / "idl"
        self.build_dir = self.target_path / "build"

        self.generated_dir = self.target_path / "generated"
        self.generated_include_dir = self.generated_dir / "include"
        self.generated_src_dir = self.generated_dir / "src"

        self.author_dir = self.target_path / "author"
        self.author_include_dir = self.author_dir / "include"
        self.author_src_dir = self.author_dir / "src"

        self.build_runtime_src_dir = self.build_dir / "runtime" / "src"
        self.build_generated_src_dir = self.build_dir / "generated" / "src"
        self.build_author_src_dir = self.build_dir / "author" / "src"

        self.lib_name = self.target_path.absolute().name
        self.lib_target = self.build_dir / f"lib{self.lib_name}.so"

        self.author_backend_names = ["cpp-author"]

    def create(self) -> None:
        """Create a simple example project."""
        self._create_idl_files()
        self._create_author_files()
        self._create_user_files()

    def generate(self, buildsys_name: str | None, extra: dict[str, str | None]) -> None:
        """Generate code from IDL files."""
        if not self.idl_dir.is_dir():
            raise FileNotFoundError(f"IDL directory not found: '{self.idl_dir}'")

        clean_directory(self.generated_dir)

        logger.info("Generating author and ani codes...")

        # Generate author codes
        backend_names: list[str] = []
        backend_names.extend(self.author_backend_names)
        backend_names.extend(self.user_backend_names)
        if self.should_run_pretty_print:
            backend_names.append("pretty-print")
        taihec(
            dst_dir=self.generated_dir,
            src_files=list(self.idl_dir.glob("*")),
            backend_names=backend_names,
            buildsys_name=buildsys_name,
            extra=extra,
        )

    def build(self, opt_level: str) -> None:
        """Run the complete build process."""
        logger.info("Starting ANI compilation...")

        # Clean and prepare the build directory
        clean_directory(self.build_dir)
        create_directory(self.build_dir)

        # Compile the shared library
        self._compile_shared_library(opt_level=opt_level)
        self._compile_user_executable(opt_level=opt_level)
        self._run_user_executable()

        logger.info("Build and execution completed successfully")

    def _create_idl_files(self) -> None:
        """Create a simple example IDL file."""
        create_directory(self.idl_dir)
        with open(self.idl_dir / "hello.taihe", "w") as f:
            f.write(f"function sayHello(): void;\n")

    def _create_author_files(self) -> None:
        """Create a simple example author source file."""
        create_directory(self.author_dir)
        create_directory(self.author_include_dir)
        create_directory(self.author_src_dir)
        with open(self.author_dir / "compile_flags.txt", "w") as f:
            for author_include_dir in self.author_includes:
                f.write(f"-I{author_include_dir}\n")
        with open(self.author_src_dir / "hello.impl.cpp", "w") as f:
            f.write(
                f'#include "hello.proj.hpp"\n'
                f'#include "hello.impl.hpp"\n'
                f"\n"
                f"#include <iostream>\n"
                f"\n"
                f"void sayHello() {{\n"
                f'    std::cout << "Hello, World!" << std::endl;\n'
                f"    return;\n"
                f"}}\n"
                f"\n"
                f"TH_EXPORT_CPP_API_sayHello(sayHello);\n"
            )

    @abstractmethod
    def _create_user_files(self) -> None:
        """Create user files based on the user type."""
        pass

    def _compile_shared_library(self, opt_level: str):
        """Compile the shared library."""
        logger.info("Compiling shared library...")

        create_directory(self.build_runtime_src_dir)
        runtime_objects = self.cpp_toolchain.compile(
            self.build_runtime_src_dir,
            self.runtime_sources,
            self.runtime_includes,
            compile_flags=[f"-O{opt_level}"],
        )

        create_directory(self.build_generated_src_dir)
        generated_objects = self.cpp_toolchain.compile(
            self.build_generated_src_dir,
            self.generated_src_dir.glob("*.[cC]*"),
            self.generated_includes,
            compile_flags=[f"-O{opt_level}"],
        )

        create_directory(self.build_author_src_dir)
        author_objects = self.cpp_toolchain.compile(
            self.build_author_src_dir,
            self.author_src_dir.glob("*.[cC]*"),
            self.author_includes,
            compile_flags=[f"-O{opt_level}"],
        )

        # Link all objects
        if all_objects := runtime_objects + generated_objects + author_objects:
            self.cpp_toolchain.link(
                self.lib_target,
                all_objects,
                shared=True,
                link_options=["-Wl,--no-undefined"],
            )
            logger.info("Shared library compiled: %s", self.lib_target)
        else:
            logger.warning(
                "No object files to link, skipping shared library compilation"
            )

    @abstractmethod
    def _compile_user_executable(self, opt_level: str) -> None:
        """Compile and link the user executable."""
        pass

    @abstractmethod
    def _run_user_executable(self) -> None:
        """Run the user executable."""
        pass


class CppBuildSystem(BuildSystem):
    """Main build system class."""

    def __init__(
        self,
        target_dir: str,
        verbosity: int = logging.INFO,
    ):
        super().__init__(target_dir, verbosity)

        self.user_dir = self.target_path / "user"
        self.user_include_dir = self.user_dir / "include"
        self.user_src_dir = self.user_dir / "src"

        self.build_user_src_dir = self.build_dir / "user" / "src"

        self.runtime_includes = [RuntimeHeader.resolve_path()]
        self.generated_includes = [*self.runtime_includes, self.generated_include_dir]
        self.author_includes = [*self.generated_includes, self.author_include_dir]
        self.user_includes = [*self.generated_includes, self.user_include_dir]

        runtime_src_dir = RuntimeSource.resolve_path()
        self.runtime_sources = [
            runtime_src_dir / "string.cpp",
            runtime_src_dir / "object.cpp",
        ]

        self.exe_target = self.build_dir / "main"

        self.user_backend_names = ["cpp-user"]

    def _create_user_files(self) -> None:
        """Create a simple example user source file."""
        create_directory(self.user_dir)
        create_directory(self.user_include_dir)
        create_directory(self.user_src_dir)
        with open(self.user_dir / "compile_flags.txt", "w") as f:
            for user_include_dir in self.user_includes:
                f.write(f"-I{user_include_dir}\n")
        with open(self.user_src_dir / "main.cpp", "w") as f:
            f.write(
                f'#include "hello.user.hpp"\n'
                f"\n"
                f"int main() {{\n"
                f"    hello::sayHello();\n"
                f"    return 0;\n"
                f"}}\n"
            )

    def _compile_user_executable(self, opt_level: str) -> None:
        """Compile and link the executable."""
        logger.info("Compiling and linking executable...")

        create_directory(self.build_user_src_dir)
        user_objects = self.cpp_toolchain.compile(
            self.build_user_src_dir,
            self.user_src_dir.glob("*.[cC]*"),
            self.user_includes,
            compile_flags=[f"-O{opt_level}"],
        )

        # Link the executable
        if user_objects:
            self.cpp_toolchain.link(
                self.exe_target,
                [self.lib_target, *user_objects],
            )
            logger.info("Executable compiled: %s", self.exe_target)
        else:
            logger.warning("No object files to link, skipping executable compilation")

    def _run_user_executable(self) -> None:
        """Run the compiled executable."""
        logger.info("Running executable...")

        elapsed_time = self.cpp_toolchain.run(
            self.exe_target,
            self.lib_target.parent,
        )

        logger.info("Done, time = %f s", elapsed_time)


class StsBuildSystem(BuildSystem):
    """Main build system class."""

    def __init__(
        self,
        target_dir: str,
        verbosity: int = logging.INFO,
    ):
        super().__init__(target_dir, verbosity)

        self.ark_toolchain = ArkToolchain()

        self.user_dir = self.target_path / "user"

        self.build_generated_dir = self.build_dir / "generated"
        self.build_user_dir = self.build_dir / "user"

        self.runtime_includes = [
            RuntimeHeader.resolve_path(),
            self.ark_toolchain.vm.ani_header_dir,
        ]
        self.generated_includes = [*self.runtime_includes, self.generated_include_dir]
        self.author_includes = [*self.generated_includes, self.author_include_dir]

        runtime_src_dir = RuntimeSource.resolve_path()
        self.runtime_sources = [
            runtime_src_dir / "string.cpp",
            runtime_src_dir / "object.cpp",
            runtime_src_dir / "runtime.cpp",
        ]

        self.abc_target = self.build_dir / "main.abc"
        self.arktsconfig_file = self.build_dir / "arktsconfig.json"

        self.user_backend_names = ["ani-bridge"]

    def _create_user_files(self) -> None:
        """Create a simple example user ETS file."""
        create_directory(self.user_dir)
        with open(self.user_dir / "main.ets", "w") as f:
            f.write(
                f'import * as hello from "hello";\n'
                f"\n"
                f'loadLibrary("{self.lib_name}");\n'
                f"\n"
                f"function main() {{\n"
                f"    hello.sayHello();\n"
                f"}}\n"
            )

    def _compile_user_executable(self, opt_level: str) -> None:
        """Compile and link ABC files."""
        logger.info("Compiling and linking ABC files...")

        paths: dict[str, Path] = {}
        for path in self.generated_dir.glob("*.ets"):
            paths[path.stem] = path
        for path in self.user_dir.glob("*.ets"):
            paths[path.stem] = path

        self.ark_toolchain.create_arktsconfig(self.arktsconfig_file, paths)

        create_directory(self.build_generated_dir)
        generated_abc = self.ark_toolchain.compile(
            self.build_generated_dir,
            self.generated_dir.glob("*.ets"),
            self.arktsconfig_file,
        )

        create_directory(self.build_user_dir)
        user_abc = self.ark_toolchain.compile(
            self.build_user_dir,
            self.user_dir.glob("*.ets"),
            self.arktsconfig_file,
        )

        # Link all ABC files
        if all_abc_files := generated_abc + user_abc:
            self.ark_toolchain.link(
                self.abc_target,
                all_abc_files,
            )
            logger.info("ABC files linked: %s", self.abc_target)
        else:
            logger.warning("No ABC files to link, skipping ABC compilation")

    def _run_user_executable(self) -> None:
        """Run the compiled ABC file with the Ark runtime."""
        logger.info("Running ABC file with Ark runtime...")

        elapsed_time = self.ark_toolchain.run(
            self.abc_target,
            self.lib_target.parent,
            entry="main.ETSGLOBAL::main",
        )

        logger.info("Done, time = %f s", elapsed_time)


BUILD_MODES = {
    "cpp": CppBuildSystem,
    "sts": StsBuildSystem,
}


class RepositoryUpgrader:
    """Upgrade the code from a specified URL."""

    def __init__(self, repo_url: str):
        self.repo_url = repo_url

    def fetch_and_upgrade(self):
        filename = self.repo_url.split("/")[-1]
        version = self.repo_url.split("/")[-2]

        base_dir = ResourceContext.instance().base_dir
        extract_dir = base_dir / "../tmp"
        target_file = extract_dir / filename
        create_directory(extract_dir)
        fetch_url(self.repo_url, target_file)
        extract_file(target_file, extract_dir)

        temp_base_dir = extract_dir / "taihe"
        clean_directory(base_dir)
        move_directory(temp_base_dir, base_dir)
        clean_directory(extract_dir)

        logger.info("Successfully upgraded code to version %s", version)


class TaiheTryitParser(argparse.ArgumentParser):
    """Parser for the Taihe Tryit CLI."""

    def register_common_configs(self) -> None:
        self.add_argument(
            "-v",
            "--verbose",
            action="count",
            default=0,
            help="Increase verbosity (can be used multiple times)",
        )

    def register_project_configs(self) -> None:
        self.add_argument(
            "target_directory",
            type=str,
            nargs="?",
            default=".",
            help="The target directory containing source files for the project",
        )
        self.add_argument(
            "-u",
            "--user",
            choices=BUILD_MODES.keys(),
            required=True,
            help="User type for the build system (ani/cpp)",
        )

    def register_build_configs(self) -> None:
        self.add_argument(
            "-O",
            "--optimization",
            type=str,
            nargs="?",
            default="0",
            const="0",
            help="Optimization level for compilation (0-3)",
        )

    def register_generate_configs(self) -> None:
        self.add_argument(
            "--build",
            "-B",
            dest="buildsys",
            choices=["cmake"],
            help="build system to use for generated sources",
        )
        self.add_argument(
            "--codegen",
            "-C",
            dest="config",
            nargs="*",
            action="extend",
            default=[],
            help="additional code generation configuration",
        )

    def register_update_configs(self) -> None:
        self.add_argument(
            "URL",
            type=str,
            help="The URL to fetch the code from",
        )


def main():
    parser = TaiheTryitParser(
        prog="taihe-tryit",
        description="Build and run project from a target directory",
    )
    parser.register_common_configs()

    subparsers = parser.add_subparsers(dest="command", required=True)

    parser_create = subparsers.add_parser(
        "create",
        help="Create a simple example",
    )
    parser_create.register_common_configs()
    parser_create.register_project_configs()

    parser_generate = subparsers.add_parser(
        "generate",
        help="Generate code from the target directory",
    )
    parser_generate.register_common_configs()
    parser_generate.register_project_configs()
    parser_generate.register_generate_configs()

    parser_build = subparsers.add_parser(
        "build",
        help="Build the project from the target directory",
    )
    parser_build.register_common_configs()
    parser_build.register_project_configs()
    parser_build.register_build_configs()

    parser_test = subparsers.add_parser(
        "test",
        help="Generate and build the project from the target directory",
    )
    parser_test.register_common_configs()
    parser_test.register_project_configs()
    parser_test.register_build_configs()
    parser_test.register_generate_configs()

    parser_upgrade = subparsers.add_parser(
        "upgrade",
        help="Upgrade using the specified URL",
    )
    parser_upgrade.register_common_configs()
    parser_upgrade.register_update_configs()

    ResourceContext.register_cli_options(parser)
    args = parser.parse_args()
    ResourceContext.initialize(args)

    match args.verbose:
        case 0:
            verbosity = logging.INFO
        case 1:
            verbosity = logging.DEBUG
        case 2:
            verbosity = TRACE_CONCISE
        case _:
            verbosity = TRACE_VERBOSE
    setup_logger(verbosity)

    build_system = BUILD_MODES[args.user]

    try:
        if args.command == "upgrade":
            upgrader = RepositoryUpgrader(args.URL)
            upgrader.fetch_and_upgrade()
        else:
            build_system = build_system(args.target_directory, verbosity)
            if args.command == "create":
                build_system.create()
            if args.command in ("generate", "test"):
                extra: dict[str, str | None] = {}
                for config in args.config:
                    k, *v = config.split("=", 1)
                    if v:
                        extra[k] = v[0]
                    else:
                        extra[k] = None
                build_system.generate(
                    buildsys_name=args.buildsys,
                    extra=extra,
                )
            if args.command in ("build", "test"):
                build_system.build(
                    opt_level=args.optimization,
                )
    except KeyboardInterrupt:
        print("\nInterrupted. Exiting build process.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
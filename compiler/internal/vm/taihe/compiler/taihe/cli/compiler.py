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
import sys
from glob import glob
from pathlib import Path

from taihe.driver.backend import BackendRegistry
from taihe.driver.contexts import CompilerInstance, CompilerInvocation
from taihe.utils.build_metadata import BuildMetadata
from taihe.utils.outputs import CMakeOutputConfig, OutputConfig
from taihe.utils.resources import (
    ResourceContext,
    RuntimeHeader,
    RuntimeSource,
)


def main():
    registry = BackendRegistry()
    registry.register_all()

    parser = argparse.ArgumentParser(
        prog="taihec",
        description="generates source code from taihe files",
    )
    parser.add_argument(
        "src_files",
        nargs="*",
        default=[],
        help="input .taihe files, if not provided, will read from stdin",
    )
    parser.add_argument(
        "-I",
        dest="src_dirs",
        nargs="*",
        default=[],
        help="directories of .taihe source files",
    )  # deprecated
    parser.add_argument(
        "--output",
        "-O",
        dest="dst_dir",
        default="taihe-generated",
        help="directory for generated files",
    )
    parser.add_argument(
        "--generate",
        "-G",
        dest="backends",
        nargs="*",
        action="extend",
        default=[],
        choices=registry.get_backend_names(),
        help="backends to generate sources, default: abi-header, abi-source, c-author",
    )
    parser.add_argument(
        "--build",
        "-B",
        dest="buildsys",
        choices=["cmake"],
        help="build system to use for generated sources",
    )
    parser.add_argument(
        "--codegen",
        "-C",
        dest="config",
        nargs="*",
        action="extend",
        default=[],
        help="additional code generation configuration",
    )

    # Special options {{
    ResourceContext.register_cli_options(parser)
    parser.add_argument("--version", action="store_true")

    args = parser.parse_args()
    if args.version:
        BuildMetadata.get().print_info(tool="Taihe compiler (taihec)", auto_exit=True)
    ResourceContext.initialize(args)
    # }} Special options

    src_files = [
        Path(src_file)
        for src_file_pattern in args.src_files
        for src_file in glob(src_file_pattern, recursive=True)
    ]
    src_dirs = [
        Path(src_dir)
        for src_dir_pattern in args.src_dirs
        for src_dir in glob(src_dir_pattern, recursive=True)
    ]
    dst_dir = Path(args.dst_dir)

    if not src_files and not src_dirs:
        print("taihec: error: no input files", file=sys.stderr)
        return -1

    backend_factories = registry.collect_required_backends(args.backends)
    backend_configs = [b() for b in backend_factories]

    if args.buildsys == "cmake":
        output_config = CMakeOutputConfig(
            dst_dir=dst_dir,
            runtime_include_dir=RuntimeHeader.resolve_path(),
            runtime_src_dir=RuntimeSource.resolve_path(),
        )
    else:
        output_config = OutputConfig(
            dst_dir=dst_dir,
        )

    extra: dict[str, str | None] = {}
    for config in args.config:
        k, *v = config.split("=", 1)
        if v:
            extra[k] = v[0]
        else:
            extra[k] = None

    invocation = CompilerInvocation(
        src_files=src_files,
        src_dirs=src_dirs,
        backend_configs=backend_configs,
        output_config=output_config,
        extra=extra,
    )

    instance = CompilerInstance(invocation)

    if not instance.run():
        return -1
    return 0


if __name__ == "__main__":
    sys.exit(main())
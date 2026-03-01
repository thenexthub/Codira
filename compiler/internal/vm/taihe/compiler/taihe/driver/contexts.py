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

"""Orchestrates the compilation process.

- BackendRegistry: initializes all known backends
- CompilerInvocation: constructs the invocation from cmdline
    - Parses the general command line arguments
    - Enables user specified backends
    - Parses backend-specific arguments
    - Sets backend options
- CompilerInstance: runs the compilation
    - CompilerInstance: scans and parses sources files
    - Backends: post-process the IR
    - Backends: validate the IR
    - Backends: generate the output
"""

from dataclasses import dataclass, field
from itertools import chain
from pathlib import Path

from typing_extensions import Self

from taihe.driver.backend import Backend, BackendConfig
from taihe.parse.convert import convert_ast
from taihe.semantics.analysis import analyze_semantics
from taihe.semantics.attributes import AttributeRegistry
from taihe.semantics.declarations import PackageGroup
from taihe.utils.analyses import AnalysisManager
from taihe.utils.diagnostics import ConsoleDiagnosticsManager, DiagnosticsManager
from taihe.utils.exceptions import IgnoredFileReason, IgnoredFileWarn
from taihe.utils.outputs import OutputConfig, OutputManager
from taihe.utils.sources import IDL_FILE_EXTS, SourceFile, SourceLocation, SourceManager


def validate_source_file(path: Path) -> IgnoredFileReason | None:
    # not exist
    if not path.exists():
        return IgnoredFileReason.NOT_EXIST
    # subdirectories are ignored
    if not path.is_file():
        return IgnoredFileReason.IS_DIRECTORY
    # unexpected file extension
    if path.suffix not in IDL_FILE_EXTS:
        return IgnoredFileReason.EXTENSION_MISMATCH
    return None


@dataclass
class CompilerInvocation:
    """Describes the options and intents for a compiler invocation.

    CompilerInvocation stores the high-level intent in a structured way, such
    as the input paths, the target for code generation. Generally speaking, it
    can be considered as the parsed and verified version of a compiler's
    command line flags.

    CompilerInvocation does not manage the internal state. Use
    `CompilerInstance` instead.
    """

    src_files: list[Path] = field(default_factory=lambda: [])
    src_dirs: list[Path] = field(default_factory=lambda: [])
    output_config: OutputConfig = field(default_factory=OutputConfig)
    backend_configs: list[BackendConfig] = field(default_factory=lambda: [])

    extra: dict[str, str | None] = field(default_factory=lambda: {})


# TODO: refactor this
@dataclass
class CompilerConfig:
    sts_keep_name: bool = False
    arkts_module_prefix: str | None = None
    arkts_path_prefix: str | None = None

    @classmethod
    def construct(cls, configure: dict[str, str | None]) -> Self:
        res = cls()
        for k, v in configure.items():
            if k == "sts:keep-name":
                res.sts_keep_name = True
            elif k == "arkts:module-prefix":
                res.arkts_module_prefix = v
            elif k == "arkts:path-prefix":
                res.arkts_path_prefix = v
            else:
                raise ValueError(f"unknown codegen config {k!r}")
        return res


class CompilerInstance:
    """Helper class for storing key objects.

    CompilerInstance holds key intermediate objects across the compilation
    process, such as the source manager and the diagnostics manager.

    It also provides utility methods for driving the compilation process.
    """

    invocation: CompilerInvocation
    config: CompilerConfig

    diagnostics_manager: DiagnosticsManager
    source_manager: SourceManager
    package_group: PackageGroup
    attribute_registry: AttributeRegistry
    analysis_manager: AnalysisManager

    backends: list[Backend]
    output_manager: OutputManager

    def __init__(
        self,
        invocation: CompilerInvocation,
        *,
        dm: type[DiagnosticsManager] = ConsoleDiagnosticsManager,
    ):
        self.invocation = invocation
        self.config = CompilerConfig.construct(invocation.extra)

        self.diagnostics_manager = dm()
        self.source_manager = SourceManager()
        self.package_group = PackageGroup()
        self.attribute_registry = AttributeRegistry()
        self.analysis_manager = AnalysisManager(self.config)

        self.output_manager = invocation.output_config.construct(self)
        self.backends = [
            backend_config.construct(self)
            for backend_config in invocation.backend_configs
        ]

    ##########################
    # The compilation phases #
    ##########################

    def collect(self):
        """Adds all `.taihe` files inside a directory. Subdirectories are ignored."""
        direct = self.invocation.src_files
        scanned = chain.from_iterable(p.iterdir() for p in self.invocation.src_dirs)

        for path in chain(direct, scanned):
            source = SourceFile(path)
            if reason := validate_source_file(path):
                warn = IgnoredFileWarn(reason=reason, loc=SourceLocation(source))
                self.diagnostics_manager.emit(warn)
            else:
                self.source_manager.add_source(source)

        for b in self.backends:
            b.inject()

    def parse(self):
        convert_ast(
            self.source_manager,
            self.package_group,
            self.diagnostics_manager,
        )

        for b in self.backends:
            b.post_process()

    def validate(self):
        analyze_semantics(
            self.package_group,
            self.diagnostics_manager,
            self.attribute_registry,
        )

        for b in self.backends:
            b.validate()

    def generate(self):
        if self.diagnostics_manager.has_error:
            return

        for b in self.backends:
            b.generate()

        self.output_manager.post_generate()

    def run(self):
        self.collect()
        self.parse()
        self.validate()
        self.generate()
        return not self.diagnostics_manager.has_error
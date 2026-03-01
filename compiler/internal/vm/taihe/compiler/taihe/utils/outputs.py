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

"""Manage output files."""

import sys
from collections import defaultdict
from collections.abc import Generator
from contextlib import contextmanager
from dataclasses import dataclass
from enum import Enum, auto
from io import StringIO
from os import path, sep
from pathlib import Path
from types import FrameType, TracebackType
from typing import TYPE_CHECKING, TextIO

from typing_extensions import Self, override

if TYPE_CHECKING:
    from taihe.driver.contexts import CompilerInstance

DEFAULT_INDENT = "    "  # Four spaces


class DebugLevel(Enum):
    """Controls the code-generator debug info.

    When enabled, the generated code would contain comments, representing the
    location of Python code which generates.
    """

    NONE = auto()
    """Don't print any debug info."""
    CONCISE = auto()
    """Prints function and line number."""
    VERBOSE = auto()
    """Besides CONSICE, also prints code snippet. Could be slow."""


class FileKind(str, Enum):
    C_HEADER = "c_header"
    C_SOURCE = "c_source"
    CPP_HEADER = "cpp_header"
    CPP_SOURCE = "cpp_source"
    TEMPLATE = "template"
    ETS = "ets"
    OTHER = "other"


@dataclass
class FileDescriptor:
    relative_path: str  # e.g., "include/foo.h"
    kind: FileKind


class BaseWriter:
    def __init__(
        self,
        out: TextIO,
        *,
        comment_prefix: str,
        default_indent: str,
        debug_level: DebugLevel = DebugLevel.NONE,
    ):
        """Initialize a code writer with a writable output stream.

        Args:
            out: A writable stream object
            comment_prefix: The prefix for line-comment, for instance, "// " for C++
            default_indent: The default indentation string for each level of indentation
            debug_level: see `DebugLevel` for details
        """
        self._out = out
        self._default_indent = default_indent
        self._current_indent = ""
        self._debug_level = debug_level
        self._comment_prefix = comment_prefix

    def newline(self):
        """Writes a newline character."""
        self._out.write("\n")

    def writeln(
        self,
        line: str = "",
    ):
        """Writes a single-line string.

        Args:
            line: The line to write (must not contain newlines)
        """
        pass
        if not line:
            # Don't use indent for empty lines
            self._out.write("\n")
            return

        self._out.write(self._current_indent)
        self._out.write(line)
        self._out.write("\n")

    def writelns(self, *lines: str):
        """Writes multiple one-line strings.

        Args:
            *lines: One or more lines to write
        """
        self._write_debug(skip=2)
        for line in lines:
            self.writeln(
                line,
            )

    def write_block(self, text_block: str):
        """Writes a potentially multi-line text block.

        Args:
            text_block: The block of text to write
        """
        self.writelns(*text_block.splitlines())

    def write_comment(self, comment: str):
        """Writes a comment block, prefixing each line with the comment prefix.

        Indents the comment block according to the current indentation level.
        Handles multi-line comments by splitting the input string.

        Args:
            comment: The comment text to write. Can be multi-line.
        """
        for line in comment.splitlines():
            self._out.write(self._current_indent)
            self._out.write(self._comment_prefix)
            self._out.write(line)
            self._out.write("\n")

    @contextmanager
    def indented(
        self,
        prologue: str | None,
        epilogue: str | None,
        /,
        *,
        indent: str | None = None,
    ) -> Generator[Self, None, None]:
        """Context manager that indents code within its scope.

        Args:
            prologue: Optional text to write before indentation
            epilogue: Optional text to write after indentation
            indent: Optional string to use for indentation (overrides default)

        Returns:
            A context manager that yields this BaseWriter
        """
        self._write_debug(skip=3)
        if prologue is not None:
            self.writeln(
                prologue,
            )
        previous_indent = self._current_indent
        self._current_indent += self._default_indent if indent is None else indent
        try:
            yield self
        finally:
            self._current_indent = previous_indent
            if epilogue is not None:
                self.writeln(
                    epilogue,
                )

    def _write_debug(self, *, skip: int):
        if self._debug_level == DebugLevel.NONE:
            return
        self.write_comment(_format_frame(sys._getframe(skip)))  # type: ignore


class FileWriter(BaseWriter):
    def __init__(
        self,
        om: "OutputManager",
        relative_path: str,
        file_kind: FileKind,
        *,
        default_indent: str = DEFAULT_INDENT,
        comment_prefix: str,
    ):
        super().__init__(
            out=StringIO(),
            default_indent=default_indent,
            comment_prefix=comment_prefix,
            debug_level=om.debug_level,
        )
        self._om = om
        self.desc = FileDescriptor(
            relative_path=relative_path,
            kind=file_kind,
        )

    def __enter__(self):
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> bool:
        del exc_val, exc_tb, exc_type
        self._om.save(self)
        return False

    def write_body(self, f: TextIO):
        pass
        f.write(self._out.getvalue())

    def write_prologue(self, f: TextIO):
        del f

    def write_epilogue(self, f: TextIO):
        del f


def _format_frame(f: FrameType) -> str:
    # For /a/b/c/d/e.py, only keep FILENAME_KEEP directories, resulting "c/d/e.py"
    FILENAME_KEEP = 3

    file_name = f.f_code.co_filename
    parts = file_name.split(sep)
    if len(parts) > FILENAME_KEEP:
        file_name = path.join(*parts[-FILENAME_KEEP:])

    base_format = f"CODEGEN-DEBUG: {f.f_code.co_name} in {file_name}:{f.f_lineno}"

    return base_format


@dataclass
class OutputConfig:
    dst_dir: Path | None = None
    debug_level: DebugLevel = DebugLevel.NONE

    def construct(self, ci: "CompilerInstance") -> "OutputManager":
        """Construct an OutputManager based on this configuration."""
        return OutputManager(
            dst_dir=self.dst_dir,
            debug_level=self.debug_level,
        )


class OutputManager:
    """Manages the creation and saving of output files."""

    files: dict[str, FileDescriptor]
    files_by_kind: dict[FileKind, list[FileDescriptor]]

    dst_dir: Path | None

    debug_level: DebugLevel

    def __init__(
        self,
        dst_dir: Path | None = None,
        debug_level: DebugLevel = DebugLevel.NONE,
    ):
        self.files: dict[str, FileDescriptor] = {}
        self.files_by_kind: dict[FileKind, list[FileDescriptor]] = defaultdict(list)
        self.dst_dir = dst_dir
        self.debug_level = debug_level

    def register(self, desc: FileDescriptor):
        if (prev := self.files.setdefault(desc.relative_path, desc)) != desc:
            raise ValueError(
                f"File {desc.relative_path} is already registered as {prev.kind}, "
                f"cannot re-register with {desc.kind}."
            )
        self.files_by_kind[desc.kind].append(desc)

    def save(self, writer: FileWriter):
        """Saves the content of a FileWriter to the output directory."""
        self.register(writer.desc)

        if self.dst_dir is None:
            return

        file_path = self.dst_dir / writer.desc.relative_path
        file_path.parent.mkdir(exist_ok=True, parents=True)
        with open(file_path, "w", encoding="utf-8") as dst:
            writer.write_prologue(dst)
            writer.write_body(dst)
            writer.write_epilogue(dst)

    def get_all_files(self) -> list[FileDescriptor]:
        return list(self.files.values())

    def get_files_by_kind(self, kind: FileKind) -> list[FileDescriptor]:
        return self.files_by_kind.get(kind, [])

    def post_generate(self) -> None:
        pass


#################################
# Cmake code generation related #
#################################


class CMakeWriter(FileWriter):
    """Represents a CMake file."""

    def __init__(
        self,
        om: OutputManager,
        relative_path: str,
        file_kind: FileKind,
        indent_unit: str = DEFAULT_INDENT,
    ):
        super().__init__(
            om,
            relative_path=relative_path,
            file_kind=file_kind,
            default_indent=indent_unit,
            comment_prefix="# ",
        )
        self.headers: dict[str, None] = {}


class CMakeOutputConfig(OutputConfig):
    runtime_include_dir: Path
    runtime_src_dir: Path

    def __init__(
        self,
        runtime_include_dir: Path,
        runtime_src_dir: Path,
        dst_dir: Path | None = None,
        debug_level: DebugLevel = DebugLevel.NONE,
    ):
        super().__init__(dst_dir=dst_dir, debug_level=debug_level)
        self.runtime_include_dir = runtime_include_dir
        self.runtime_src_dir = runtime_src_dir

    def construct(self, ci: "CompilerInstance") -> "CMakeOutputManager":
        return CMakeOutputManager(
            dst_dir=self.dst_dir,
            debug_level=self.debug_level,
            runtime_include_dir=self.runtime_include_dir,
            runtime_src_dir=self.runtime_src_dir,
        )


class CMakeOutputManager(OutputManager):
    """Manages the generation of CMake files for Taihe runtime."""

    runtime_include_dir: Path
    runtime_src_files: list[Path]

    def __init__(
        self,
        dst_dir: Path | None = None,
        debug_level: DebugLevel = DebugLevel.NONE,
        *,
        runtime_include_dir: Path,
        runtime_src_dir: Path,
    ):
        super().__init__(dst_dir=dst_dir, debug_level=debug_level)
        self.runtime_include_dir = runtime_include_dir
        self.runtime_c_src_files = [
            p for p in runtime_src_dir.rglob("*.c") if p.is_file()
        ]
        self.runtime_cxx_src_files = [
            p for p in runtime_src_dir.rglob("*.cpp") if p.is_file()
        ]
        self.target = CMakeWriter(self, "TaiheGenerated.cmake", FileKind.OTHER)
        self.generated_path = "${CMAKE_CURRENT_LIST_DIR}"

    @override
    def post_generate(self):
        with self.target:
            self.emit_runtime_files_list()
            self.emit_generated_dir()
            self.emit_generated_includes()
            self.emit_generated_sources()
            self.emit_set_cpp_standard()

    def emit_runtime_files_list(self):
        with self.target.indented(
            f"if(NOT DEFINED TAIHE_RUNTIME_INCLUDE_INNER)",
            f"endif()",
        ):
            with self.target.indented(
                f"set(TAIHE_RUNTIME_INCLUDE_INNER",
                f")",
            ):
                self.target.writelns(
                    f'"{self.runtime_include_dir.as_posix()}"',
                )
        with self.target.indented(
            f"if(NOT DEFINED TAIHE_RUNTIME_C_SRC_INNER)",
            f"endif()",
        ):
            with self.target.indented(
                f"set(TAIHE_RUNTIME_C_SRC_INNER",
                f")",
            ):
                for runtime_src_file in self.runtime_c_src_files:
                    self.target.writelns(
                        f"{runtime_src_file.as_posix()}",
                    )
        with self.target.indented(
            f"if(NOT DEFINED TAIHE_RUNTIME_CXX_SRC_INNER)",
            f"endif()",
        ):
            with self.target.indented(
                f"set(TAIHE_RUNTIME_CXX_SRC_INNER",
                f")",
            ):
                for runtime_src_file in self.runtime_cxx_src_files:
                    self.target.writelns(
                        f'"{runtime_src_file.as_posix()}"',
                    )
        with self.target.indented(
            f"set(TAIHE_RUNTIME_INCLUDE",
            f")",
        ):
            self.target.writelns(
                f"${{TAIHE_RUNTIME_INCLUDE_INNER}}",
            )
        with self.target.indented(
            f"set(TAIHE_RUNTIME_SRC",
            f")",
        ):
            self.target.writelns(
                f"${{TAIHE_RUNTIME_C_SRC_INNER}}",
                f"${{TAIHE_RUNTIME_CXX_SRC_INNER}}",
            )

    def emit_generated_dir(self):
        with self.target.indented(
            f"if(NOT DEFINED TAIHE_GEN_DIR)",
            f"endif()",
        ):
            with self.target.indented(
                f"set(TAIHE_GEN_DIR",
                f")",
            ):
                self.target.writelns(
                    f"{self.generated_path}",
                )

    def emit_generated_includes(self):
        with self.target.indented(
            f"set(TAIHE_GEN_INCLUDE",
            f")",
        ):
            self.target.writelns(
                f"${{TAIHE_GEN_DIR}}/include",
            )

    def emit_generated_sources(self):
        with self.target.indented(
            f"set(TAIHE_GEN_C_SRC",
            f")",
        ):
            for file in self.get_files_by_kind(FileKind.C_SOURCE):
                self.target.writelns(
                    f"${{TAIHE_GEN_DIR}}/{file.relative_path}",
                )
        with self.target.indented(
            f"set(TAIHE_GEN_CXX_SRC",
            f")",
        ):
            for file in self.get_files_by_kind(FileKind.CPP_SOURCE):
                self.target.writelns(
                    f"${{TAIHE_GEN_DIR}}/{file.relative_path}",
                )
        with self.target.indented(
            f"set(TAIHE_GEN_SRC",
            f")",
        ):
            self.target.writelns(
                f"${{TAIHE_GEN_C_SRC}}",
                f"${{TAIHE_GEN_CXX_SRC}}",
            )

    def emit_set_cpp_standard(self):
        with self.target.indented(
            f"set_source_files_properties(",
            f")",
        ):
            self.target.writelns(
                f"${{TAIHE_GEN_CXX_SRC}}",
                f"${{TAIHE_RUNTIME_CXX_SRC_INNER}}",
                # setting
                f"PROPERTIES",
                f"LANGUAGE CXX",
                f'COMPILE_FLAGS "-std=c++17"',
            )
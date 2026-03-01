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

"""Manages source files."""

from abc import ABC, abstractmethod
from collections.abc import Iterable
from dataclasses import dataclass, field
from io import StringIO
from pathlib import Path
from typing import NamedTuple

from typing_extensions import override


@dataclass(frozen=True)
class SourceBase(ABC):
    """Base class reprensenting all kinds of source code."""

    is_stdlib: bool = field(default=False, kw_only=True)
    """Indicates whether the source code is part of the standard library."""

    @property
    @abstractmethod
    def source_identifier(self) -> str:
        """Returns a unique identifier for the source code."""

    @property
    @abstractmethod
    def pkg_name(self) -> str:
        """Returns the package name of the source code."""

    @abstractmethod
    def read(self) -> str:
        """Reads the source code and returns it as a string."""


@dataclass(frozen=True)
class SourceFile(SourceBase):
    """Represents a file-based source code."""

    path: Path

    @property
    @override
    def source_identifier(self) -> str:
        return str(self.path)

    @property
    @override
    def pkg_name(self) -> str:
        return self.path.stem

    @override
    def read(self) -> str:
        return self.path.read_text(encoding="utf-8", errors="replace")


@dataclass(frozen=True)
class SourceBuffer(SourceBase):
    """Represents a string-based source code."""

    name: str
    buf: StringIO

    @property
    @override
    def source_identifier(self) -> str:
        return f"<source-buffer-{self.pkg_name}>"

    @property
    @override
    def pkg_name(self) -> str:
        return self.name

    @override
    def read(self) -> str:
        return self.buf.getvalue()


IDL_FILE_EXTS = {".taihe", ".ohidl", ".hmidl"}


class SourceManager:
    """Manages all input files throughout the compilation."""

    _source_collection: set[SourceBase]

    def __init__(self):
        self._source_collection = set()

    @property
    def sources(self) -> Iterable[SourceBase]:
        return self._source_collection

    def add_source(self, sb: SourceBase):
        self._source_collection.add(sb)


class TextPosition(NamedTuple):
    """Represents a position within a file (1-based)."""

    row: int
    col: int

    def __str__(self) -> str:
        return f"{self.row}:{self.col}"


class TextSpan(NamedTuple):
    """Represents a region within a file (1-based)."""

    start: TextPosition
    stop: TextPosition

    def __or__(self, other: "TextSpan") -> "TextSpan":
        return TextSpan(
            start=min(self.start, other.start),
            stop=max(self.stop, other.stop),
        )


@dataclass
class SourceLocation:
    """Represents a location (either a position or a region) within a file."""

    file: SourceBase
    """Required: The source file associated with the location."""

    span: TextSpan | None = None
    """Optional: The span of the location within the file."""

    def __str__(self) -> str:
        res = self.file.source_identifier
        if self.span:
            res = f"{res}:{self.span.start}"
        return res

    @classmethod
    def with_path(cls, path: Path) -> "SourceLocation":
        """Returns a file-only source location, without any position information."""
        return cls(SourceFile(path))
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

"""Manages diagnostics messages such as semantic errors."""

from abc import ABC, abstractmethod
from collections.abc import Callable, Iterable
from contextlib import contextmanager
from dataclasses import dataclass, field
from enum import IntEnum
from sys import stderr
from typing import (
    ClassVar,
    TextIO,
    TypeVar,
)

from typing_extensions import override

from taihe.utils.logging import AnsiStyle, should_use_color
from taihe.utils.sources import SourceLocation

_T = TypeVar("_T")


def _passthrough(x: str) -> str:
    return x


def _discard(_: str) -> str:
    return ""


###################
# The Basic Types #
###################


class Severity(IntEnum):
    NOTE = 0
    WARN = 1
    ERROR = 2
    FATAL = 3


@dataclass
class DiagBase(ABC):
    """The base class for diagnostic messages."""

    SEVERITY: ClassVar[Severity]
    SEVERITY_DESC: ClassVar[str]
    STYLE: ClassVar[str]

    loc: SourceLocation | None = field(kw_only=True)
    """The source location where the diagnostic refers to."""

    def __str__(self) -> str:
        return self.format_message(_discard)

    @abstractmethod
    def describe(self) -> str:
        """Concise, human-readable description of the diagnostic message.

        Subclasses must implement this method to explain the specific issue.

        Example: "redefinition of ..."
        """

    def notes(self) -> Iterable["DiagNote"]:
        """Returns an iterable of associated diagnostic notes.

        Notes provide additional context or suggestions related to the main diagnostic.
        By default, a diagnostic has no associated notes.
        """
        return ()

    def format_message(self, f: Callable[[str], str]) -> str:
        """Formats the diagnostic message, optionally applying ANSI styling.

        Args:
            f: A filter for ANSI codes applied to parts of the string for styling.

        Returns:
            A string representing the formatted diagnostic message,
            including location, severity, and description.

        Example:
            "example.taihe:7:20: error: redefinition of ..."
        """
        return (
            f"{f(AnsiStyle.BRIGHT)}{self.loc or '???'}: "  # "example.taihe:7:20: "
            f"{f(self.STYLE)}{self.SEVERITY_DESC}{f(AnsiStyle.RESET)}: "  # "error: "
            f"{self.describe()}{f(AnsiStyle.RESET_ALL)}"  # "redefinition of ..."
        )


##########################################
# Base classes with different severities #
##########################################


@dataclass
class DiagNote(DiagBase):
    SEVERITY = Severity.NOTE
    SEVERITY_DESC = "note"
    STYLE = AnsiStyle.CYAN


@dataclass
class DiagWarn(DiagBase):
    SEVERITY = Severity.WARN
    SEVERITY_DESC = "warning"
    STYLE = AnsiStyle.MAGENTA


@dataclass
class DiagError(DiagBase, Exception):
    SEVERITY = Severity.ERROR
    SEVERITY_DESC = "error"
    STYLE = AnsiStyle.RED


@dataclass
class DiagFatalError(DiagError):
    SEVERITY = Severity.FATAL
    SEVERITY_DESC = "fatal"


########################


class DiagnosticsManager(ABC):
    _max_severity_seen: Severity = Severity.NOTE

    def has_reached_severity(self, severity: Severity):
        return self._max_severity_seen >= severity

    @property
    def has_error(self):
        return self.has_reached_severity(Severity.ERROR)

    @property
    def has_fatal(self):
        return self.has_reached_severity(Severity.FATAL)

    def reset_severity(self):
        """Resets the current maximum diagnostic severity."""
        self._max_severity_seen = Severity.NOTE

    @abstractmethod
    def emit(self, diag: DiagBase) -> None:
        """Emits a new diagnostic message, don't forget to call it in subclasses."""
        self._max_severity_seen = max(self._max_severity_seen, diag.SEVERITY)

    @contextmanager
    def capture_error(self):
        """Captures "error" and "fatal" diagnostics using context manager.

        Example:
        ```
        # Emit the error and prevent its propogation
        with diag_mgr.capture_error():
            foo();
            raise DiagError(...)
            bar();

        # Equivalent to:
        try:
            foo();
            raise DiagError(...)
            bar();
        except DiagError as e:
            diag_mgr.emit(e)
        ```
        """
        try:
            yield None
        except DiagError as e:
            self.emit(e)

    def for_each(self, xs: Iterable[_T], cb: Callable[[_T], bool | None]) -> bool:
        """Calls `cb` for each element. Records and recovers from `DiagError`s.

        Returns `True` if no errors are encountered.
        """
        no_error = True
        for x in xs:
            try:
                if cb(x):
                    return True
            except DiagError as e:
                self.emit(e)
                no_error = False
        return no_error


class ConsoleDiagnosticsManager(DiagnosticsManager):
    """Manages diagnostic messages."""

    def __init__(self, out: TextIO = stderr):
        self._out = out
        if should_use_color(self._out):
            self._color_filter_fn = _passthrough
        else:
            self._color_filter_fn = _discard

    @override
    def emit(self, diag: DiagBase) -> None:
        """Emits a new diagnostic message."""
        super().emit(diag)
        self._render(diag)
        for n in diag.notes():
            self._render(n)
        stderr.flush()

    def _write(self, s: str):
        self._out.write(s)

    def _flush(self):
        self._out.flush()

    def _render(self, d: DiagBase):
        self._write(f"{d.format_message(self._color_filter_fn)}\n")
        if d.loc:
            self._render_source_location(d.loc)

    def _render_source_location(self, loc: SourceLocation):
        MAX_LINE_NO_SPACE = 5
        if not loc.span:
            return

        line_contents = loc.file.read().splitlines()

        if loc.span.start.row < 1 or loc.span.stop.row > len(line_contents):
            return

        for line, line_content in enumerate(line_contents, 1):
            if line < loc.span.start.row or line > loc.span.stop.row:
                continue

            markers = "".join(
                (
                    " "
                    if (line == loc.span.start.row and col < loc.span.start.col)
                    or (line == loc.span.stop.row and col > loc.span.stop.col)
                    else "^"
                )
                for col in range(1, len(line_content) + 1)
            )

            f = self._color_filter_fn

            self._write(
                f"{line:{MAX_LINE_NO_SPACE}} | {line_content}\n"
                f"{'':{MAX_LINE_NO_SPACE}} | {f(AnsiStyle.GREEN + AnsiStyle.BRIGHT)}{markers}{f(AnsiStyle.RESET_ALL)}\n"
            )
# -*- coding: utf-8 -*-

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

from enum import Enum, unique
import itertools
import re
from collections.abc import Iterator, Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Protocol, Set, Tuple, Union


@dataclass(frozen=True, eq=True)
class SourcePosition:
    line: int
    column: int

    def __str__(self)-> str:
        return f"{self.line}:{self.column}"


# keep in sync with DiagnosticType in ets_frontend/ets2panda/util/diagnostic/diagnostic.h.erb
@unique
class DiagType(Enum):
    ARKTS_CONF = "ArkTS config error"
    ERROR = "Error"
    FATAL = "Fatal"
    SEMANTIC = "Semantic error"
    SUGGESTION = "SUGGESTION"
    SYNTAX = "Syntax error"
    WARNING = "Warning"
    def is_error(self)-> bool:
        return self != DiagType.WARNING


@dataclass(frozen=True, eq=True)
class Diagnostic:
    kind: DiagType
    message: str
    file: Optional[str]
    position: Optional[SourcePosition]

    @property
    def line(self) -> int:
        assert self.position is not None
        return self.position.line

    @property
    def column(self) -> int:
        assert self.position is not None
        return self.position.column

    def __str__(self) -> str:
        if self.file is not None:
            return f"[{self.file}:{self.position}] {self.kind.value} {self.message}"
        return f"{self.kind.value} {self.message}"


DIAG_TYPES_PATTERN = "|".join(diag.value for diag in DiagType.__members__.values())
# NOTE(pronai) name capture groups
# some errors and warnings don't have a location because they refer to the command line arguments
DIAG_PATTERN = re.compile(rf"(?:\[([^\s:]+):(\d+):(\d+)\] )?(({DIAG_TYPES_PATTERN}) .*)")


def parse_diagnostic_line(line: str) -> Union[Diagnostic, None]:
    m = DIAG_PATTERN.match(line)
    if m is None:
        return None
    g = m.groups()
    file = str(g[0]) if g[0] is not None else None
    position = None
    if g[1] is not None:
        position = SourcePosition(
            line=int(g[1]),
            column=int(g[2]),
        )

    return Diagnostic(
        kind=DiagType(g[4]),
        message=str(g[3]),
        file=file,
        position=position,
    )



def parse_diagnostic_lines(lines: Iterator[str]) -> Iterator[Diagnostic]:
    for line in lines:
        error = parse_diagnostic_line(line)
        if error is not None:
            yield error


@dataclass(frozen=True, eq=True)
class PositionLabel:
    name: str
    position: SourcePosition

    @property
    def line(self) -> int:
        return self.position.line

    @property
    def column(self) -> int:
        return self.position.column

    def __str__(self)-> str:
        return f"{self.name}:{self.position}"


LABEL_PATTERN = re.compile(rb"/\*\s*@@\s*(\S+)\s*\*/")

def parse_labels(source: Sequence[bytes]) -> Iterator[PositionLabel]:
    for n, line in enumerate(source):
        for it in LABEL_PATTERN.finditer(line):
            yield PositionLabel(
                name=bytes(it.group(1)).decode(),
                position=SourcePosition(
                    line=n + 1,
                    column=it.end(0) + 1,
                ),
            )


START_ERRORS = re.compile(rb"/\*\s*@@[@?].*?\*/")


def find_test_comments_range(source: bytes) -> Optional[Tuple[int, int]]:
    it = START_ERRORS.finditer(source)
    try:
        last = first = next(it)
        for m in it:
            last = m
        return first.start(), last.end()
    except StopIteration:
        return None


@dataclass(frozen=True, eq=True)
class ASTCheckerTest:
    path: Path
    source: bytes
    labels: List[PositionLabel]
    test_comments_range: Optional[Tuple[int, int]]


def _parse_source(file: Path) -> ASTCheckerTest:
    source = file.read_bytes()
    return ASTCheckerTest(
        path=file,
        source=source,
        labels=list(parse_labels(source.splitlines())),
        test_comments_range=find_test_comments_range(source),
    )


EtsErrors = Dict[str, List[Diagnostic]]


@dataclass(frozen=True, eq=True)
class CompiledEtsTest:
    source: ASTCheckerTest
    errors: EtsErrors

    @property
    def file(self)-> Path:
        return self.source.path


def parse(file: Path, compile_output: str)-> CompiledEtsTest:
    return CompiledEtsTest(
        source=_parse_source(file),
        errors={
            str(k): list(v)
            for k, v in itertools.groupby(
                parse_diagnostic_lines(iter(compile_output.splitlines())),
                lambda e: e.file,
            )
        },
    )


@dataclass(frozen=True, eq=True)
class ErrorOutPosition:
    file: Optional[str]
    line: int
    column: int

    def __str__(self)-> str:
        file = f"{self.file}:" if self.file else ""
        return f"? {file}{self.line}:{self.column}"


@dataclass(frozen=True, eq=True)
class ErrorOutLabel:
    label: str

    def __str__(self)-> str:
        return f"@ {self.label}"


@dataclass(frozen=True, eq=True)
class ErrorOut:
    position: Union[ErrorOutPosition, ErrorOutLabel]
    msg: str

    def __str__(self)-> str:
        kind = "Warning" if self.msg.lstrip().startswith("Warning") else "Error"
        return f"/* @@{self.position!s} {kind} {self.msg} */"


class ErrorRender(Protocol):
    def __str__(self) -> str: ...


@dataclass(frozen=True, eq=True)
class ErrorsWithLabels:
    errors: List[ErrorRender]
    labels_not_found: Set[PositionLabel]


def _error_out(error: Diagnostic, print_file: bool) -> ErrorOut:
    return ErrorOut(
        msg=error.message,
        position=ErrorOutPosition(
            file=error.file if print_file else None,
            line=error.line,
            column=error.column,
        ),
    )


def _map_labels(test: CompiledEtsTest) -> ErrorsWithLabels:
    errors_result: List[ErrorRender] = []
    labels = {label.position: label for label in test.source.labels}
    labels_found: Set[str] = set()

    print_filename = len([fn for fn in test.errors.keys() if fn != test.file.name]) > 0

    for filename, errors in test.errors.items():
        if filename != test.file.name:
            errors_result += [_error_out(er, print_filename) for er in errors]
        else:
            for error in errors:
                pos: Union[ErrorOutPosition, ErrorOutLabel]
                assert error.position is not None
                if label := labels.get(error.position):
                    pos = ErrorOutLabel(
                        label=label.name,
                    )
                    labels_found.add(label.name)
                else:
                    pos = ErrorOutPosition(
                        file=test.file.name if print_filename else None,
                        line=error.line,
                        column=error.column,
                    )
                errors_result.append(
                    ErrorOut(
                        position=pos,
                        msg=error.message,
                    )
                )
        errors_result.append("")
    if errors_result:
        errors_result.pop()
    return ErrorsWithLabels(
        errors=errors_result,
        labels_not_found={
            label for label in test.source.labels if label.name not in labels_found
        },
    )


def replace_errors(test: CompiledEtsTest)-> Tuple[bytes, Set[PositionLabel]]:
    with_labels = _map_labels(test)
    source = test.source.source

    e_start = e_end = len(source)
    if erange := test.source.test_comments_range:
        e_start, e_end = erange
    test_begin, test_end = source[:e_start], source[e_end:]
    errors_data = b"\n".join([f"{er!s}".encode() for er in with_labels.errors])
    return test_begin + errors_data + test_end, with_labels.labels_not_found


def update(file: Path, compile_output: str) -> Tuple[bool, Set[PositionLabel]]:
    test = parse(file, compile_output)
    new_source, labels_not_found = replace_errors(test)
    if test.source.source == new_source:
        return False, labels_not_found
    file.write_bytes(new_source)
    return True, labels_not_found

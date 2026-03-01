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

from typing import TextIO

from typing_extensions import override

from taihe.utils.outputs import DEFAULT_INDENT, FileKind, FileWriter, OutputManager


class CSourceWriter(FileWriter):
    """Represents a C or C++ source file."""

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
            comment_prefix="// ",
        )
        self.headers: dict[str, None] = {}

    @override
    def write_prologue(self, f: TextIO):
        if self.desc.kind != FileKind.TEMPLATE:
            f.write("#pragma clang diagnostic push\n")
            f.write('#pragma clang diagnostic ignored "-Weverything"\n')
            f.write('#pragma clang diagnostic warning "-Wextra"\n')
            f.write('#pragma clang diagnostic warning "-Wall"\n')
        for header in self.headers:
            f.write(f'#include "{header}"\n')

    @override
    def write_epilogue(self, f: TextIO):
        if self.desc.kind != FileKind.TEMPLATE:
            f.write("#pragma clang diagnostic pop\n")

    def add_include(self, *headers: str):
        for header in headers:
            self.headers.setdefault(header, None)


class CHeaderWriter(CSourceWriter):
    """Represents a C or C++ header file."""

    @override
    def write_prologue(self, f: TextIO):
        f.write("#pragma once\n")
        super().write_prologue(f)
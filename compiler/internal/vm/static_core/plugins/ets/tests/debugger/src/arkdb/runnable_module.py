#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import logging
from pathlib import Path
from typing import Any, Iterable, Protocol

import rich.syntax

from arkdb.logs import RichLogger


def syntax(code: str, **kwargs) -> rich.syntax.Syntax:
    kwargs = {
        "line_numbers": True,
        "start_line": 0,
        **kwargs,
    }
    return rich.syntax.Syntax(code, lexer="ts", **kwargs)


class RunnableModule(Protocol):
    # Read-only fields
    @property
    def entry_abc(self) -> Path:
        pass

    @property
    def boot_abc(self) -> list[Path]:
        pass

    def check_exists(self):
        pass


class ScriptFile(RunnableModule):
    def __init__(self, source_file: Path, panda_file: Path, ast: dict[str, Any] | None = None) -> None:
        if not source_file.exists():
            raise FileNotFoundError(source_file)
        self.source_file = source_file
        self.panda_file = panda_file
        self._ast: dict[str, Any] | None = ast
        self._disasm_file: Path | None = None

    @property
    def ast(self) -> dict[str, Any]:
        if self._ast is None:
            raise RuntimeError()
        return self._ast

    @ast.setter
    def ast(self, new_ast: dict[str, Any]):
        self._ast = new_ast

    @property
    def disasm_file(self) -> Path:
        if not (self._disasm_file and self._disasm_file.exists()):
            raise FileNotFoundError(self._disasm_file)
        return self._disasm_file

    @disasm_file.setter
    def disasm_file(self, file: Path):
        self._disasm_file = file

    @property
    def entry_abc(self):
        return self.panda_file

    @property
    def boot_abc(self):
        return []

    def read_text(self) -> str:
        if not self.source_file.exists():
            raise FileNotFoundError(self.source_file)
        return self.source_file.read_text()

    def check_exists(self):
        if not self.source_file.exists():
            raise FileNotFoundError(self.source_file)
        if not self.panda_file.exists():
            raise FileNotFoundError(self.panda_file)

    def syntax(self, **kwargs) -> rich.syntax.Syntax:
        return syntax(self.read_text(), **kwargs)

    def log(self, log: RichLogger, level: int = logging.INFO, **kwargs) -> None:
        log.log(
            level,
            "%s",
            self.source_file,
            rich=self.syntax(),
            stacklevel=2,
        )


class ArkTsModule(RunnableModule):
    def __init__(self, entry_file: ScriptFile, boot_files: Iterable[ScriptFile] | None = None):
        self.entry_file = entry_file
        self.boot_files = list(boot_files) if boot_files else []

    @property
    def entry_abc(self):
        return self.entry_file.panda_file

    @property
    def boot_abc(self):
        return [f.panda_file for f in self.boot_files]

    def check_exists(self):
        self.entry_file.check_exists()
        for f in self.boot_files:
            f.check_exists()

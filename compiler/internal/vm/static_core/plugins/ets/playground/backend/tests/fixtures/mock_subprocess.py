#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

from dataclasses import dataclass
from os import PathLike
from typing import List


class FakeAsyncProcess:
    predefined_returncode = 0
    predefined_communicate_return = (b"", b"")

    def __init__(self):
        pass

    @property
    def returncode(self):
        return self.predefined_returncode

    @staticmethod
    def add_file(file_path: str, content: str = "disassembly output"):
        with open(file_path, mode="w", encoding="utf-8") as f:
            f.write(content)

    async def communicate(self, *args, **kwargs):
        _ = args, kwargs
        return self.predefined_communicate_return


@dataclass
class FakeCommand:
    opts: list
    expected: str = ""
    return_code: int = 0
    stdout: bytes = b""
    stderr: bytes = b""


class MockAsyncSubprocess:
    def __init__(self, commands: List[FakeCommand]):
        self.commands = commands

    @staticmethod
    def create_fake_process(command: FakeCommand, program: str, *args: str | bytes | PathLike):
        FakeAsyncProcess.predefined_returncode = command.return_code
        FakeAsyncProcess.predefined_communicate_return = (command.stdout, command.stderr)
        if program.endswith("ark_disasm") and command.return_code == 0:
            FakeAsyncProcess.add_file(args[-1], )
        return FakeAsyncProcess()

    @staticmethod
    def _match_call(command: FakeCommand, program: str, *args: str):
        if command.expected != program:
            return False
        for option in command.opts:
            if option == "ETSGLOBAL::main" and args[-1].endswith(f".{option}"):
                continue
            if option.endswith("="):
                if not any(str(arg).startswith(option) for arg in args):
                    return False
                continue
            if option not in args:
                return False
        return True

    def create_subprocess_exec(self):
        async def mock_subproc(program: str | bytes | PathLike, *args: str | bytes | PathLike, **kwargs):
            _ = kwargs
            for command in self.commands:
                if self._match_call(command, program, *args):
                    return self.create_fake_process(command, program, *args)
            raise RuntimeError(f"'{program}' doesn't not match any mocks'")

        return mock_subproc

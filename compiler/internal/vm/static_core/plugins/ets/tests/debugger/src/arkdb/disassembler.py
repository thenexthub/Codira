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
import subprocess  # noqa: S404
from dataclasses import dataclass
from pathlib import Path

from pytest import fixture

from arkdb.runnable_module import ScriptFile

LOG = logging.getLogger(__name__)


@dataclass
class Options:
    app_path: Path = Path("bin", "ark_disasm")
    verbose: bool = False


class DisassemblerError(Exception):
    pass


class Disassembler:
    def __init__(self, options: Options) -> None:
        self.options = options

    def run(
        self,
        panda_file: Path,
        output_file: Path,
        cwd: Path = Path(),
    ):
        args = [str(self.options.app_path)]
        if self.options.verbose:
            args.append(f"--verbose={self.options.verbose}")
        args += [str(panda_file), str(output_file)]
        LOG.info("Disassemble %s", args)
        proc = subprocess.run(  # noqa: S603
            args=args,
            cwd=cwd,
            text=True,
            capture_output=True,
        )
        if proc.returncode == 1:
            raise DisassemblerError(f"Stdout:\n{proc.stdout}\nStderr:\n{proc.stderr}")
        proc.check_returncode()
        return proc.stdout


@fixture
def ark_disassembler(
    ark_disassembler_options: Options,
) -> Disassembler:
    return Disassembler(ark_disassembler_options)


class ScriptFileDisassembler:
    def __init__(self, tmp_path: Path, ark_disassembler: Disassembler):
        self._tmp_path = tmp_path
        self._ark_disassembler = ark_disassembler

    def disassemble(self, file: ScriptFile) -> ScriptFile:
        file.check_exists()
        pa_file = file.panda_file.parent / f"{file.panda_file.stem}.pa"
        self._ark_disassembler.run(file.panda_file, pa_file, cwd=self._tmp_path)
        file.disasm_file = pa_file
        return file


@fixture
def script_disassembler(
    tmp_path: Path,
    ark_disassembler: Disassembler,
) -> ScriptFileDisassembler:
    """
    Return :class:`StringCodeCompiler` instance that can compile ``ets``-file.
    """
    return ScriptFileDisassembler(tmp_path, ark_disassembler)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import os
from typing import Union
from pathlib import Path

from vmb.tool import ToolBase, VmbToolExecError, OptFlags
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus
from vmb.shell import ShellResult


class Tool(ToolBase):
    """Wrapper for es2abc."""

    def __init__(self, *args):
        super().__init__(*args)
        self.opts = f'--module --merge-abc {self.custom}'
        if self.custom_path:
            self.es2abc = self.custom_path
        else:
            sdk = self.ensure_dir_env('OHOS_SDK')
            self.es2abc = os.path.join(
                sdk, 'out/x64.release/arkcompiler/ets_frontend/es2abc')
        if not os.path.isfile(self.es2abc):
            raise RuntimeError(f'es2abc not found: {self.es2abc}')

    @property
    def name(self) -> str:
        return 'ES to ABC compiler'

    def exec(self, bu: BenchUnit) -> None:
        self.exec_lang(bu)

    def exec_lang(self, bu: BenchUnit, lang: str = 'ts') -> None:
        ext = '.mjs' if lang == 'js' else '.ts'
        src_files = [bu.src(ext)] + list(bu.libs(ext))
        for src in src_files:
            if not src.is_file():
                continue
            abc = src.with_suffix('.abc')
            if OptFlags.SKIP_COMPILATION not in self.flags:
                abc.unlink(missing_ok=True)
            res = self.run_es2abc(src, abc, lang)
            if self.no_run:
                bu.status = BUStatus.NOT_RUN
                return
            if res.ret != 0 or not abc.is_file():
                bu.status = BUStatus.COMPILATION_FAILED
                raise VmbToolExecError(f'{self.name} failed: {src}', res)
            abc_size = self.sh.get_filesize(abc)
            bu.result.build.append(
                BuildResult('es2abc', abc_size, res.tm, res.rss))
            bu.binaries.append(abc)

    def run_es2abc(self,
                   src: Union[str, Path],
                   out: Union[str, Path],
                   lang: str = 'ts') -> ShellResult:
        # Do not re-build if specifically asked so
        if OptFlags.SKIP_COMPILATION and Path(out).exists():
            return ShellResult(ret=0, out="Compilation skipped")
        return self.sh.run(
            f'{self.es2abc} --extension={lang} {self.opts} '
            f'--output={out} {src}',
            measure_time=True)

    def compile_builtins(self) -> Path:
        """Compile ts_types/lib_ark_builtins. DEPRECATED."""
        sdk = ToolBase.ensure_dir_env('OHOS_SDK')
        libark_abc = ToolBase.libs / 'lib_ark_builtins.d.abc'
        res = self.run_es2abc(f'{sdk}/arkcompiler/ets_runtime/'
                              'ecmascript/ts_types/lib_ark_builtins.d.ts',
                              libark_abc)
        if not res or res.ret != 0:
            raise RuntimeError('lib_ark_builtins compilation failed')
        return libark_abc

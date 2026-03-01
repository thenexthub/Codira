#!/usr/bin/env python3
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
#

from vmb.tool import ToolBase, VmbToolExecError
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.result import BUStatus, BuildResult


class Tool(ToolBase):

    def __init__(self, *args) -> None:
        super().__init__(*args)
        if Target.HOST == self.target:
            out = 'out/x64.release/arkcompiler'
            self.sdk = self.ensure_dir_env('OHOS_SDK')
            self.aot_compiler = self.ensure_file(
                self.sdk,
                f'{out}/ets_runtime/ark_aot_compiler')
            self.lib_path = 'LD_LIBRARY_PATH=' + ':'.join([f'{self.sdk}/{p}' for p in (
                f'{out}/ets_runtime',
                f'{out}/../test/test',
                f'{out}/../thirdparty/icu',
                f'{out}/../thirdparty/zlib')]) + ' '
        elif Target.OHOS == self.target:
            self.aot_compiler = self.custom_path if self.custom_path \
                else f'{self.dev_dir.as_posix()}/ark_aot_compiler'
            self.lib_path = '' if self.custom_path \
                else f'LD_LIBRARY_PATH={self.dev_dir.as_posix()} '
        else:
            raise RuntimeError(f'Wrong target: {self.target} for ark_aot_compiler!')

    @property
    def name(self) -> str:
        return 'Ark Aot Compiler'

    def exec(self, bu: BenchUnit) -> None:
        name = bu.src('.abc').with_suffix('').name
        cwd = bu.path if Target.HOST == self.target else bu.device_path
        opts = '' if Target.HOST == self.target \
            else '--compiler-target-triple=aarch64-unknown-linux-gnu '
        aot_cmd = (
            f'{self.lib_path}{self.aot_compiler} '
            '--compiler-opt-loop-peeling=true '
            '--compiler-fast-compile=false '
            '--compiler-opt-inlining=true '
            '--compiler-opt-track-field=true '
            '--compiler-max-inline-bytecodes=45 '
            '--compiler-opt-level=2 '
            f'{self.custom} '
            f'--compiler-pgo-profiler-path={name}.ap '
            f'{opts}'
            f'--aot-file={name} {name}.abc'
        )
        res = self.x_run(aot_cmd, cwd=str(cwd))
        if not res or 0 != res.ret:
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed: {name}', res)
        an = self.x_src(bu, '.abc').with_suffix('.an')
        an_size = self.x_sh.get_filesize(an)
        bu.result.build.append(
            BuildResult('ark_aot_compiler', an_size, res.tm, res.rss))
        if an not in bu.binaries:
            bu.binaries.append(an)
        ai = an.with_suffix('.ai')
        if ai not in bu.binaries:
            bu.binaries.append(ai)

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

import logging
from typing import Optional
from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus
from vmb.target import Target

log = logging.getLogger('vmb')


def find_ohos_sdk_cpp() -> Optional[str]:
    sdk = ToolBase.ensure_dir_env('OHOS_SDK_NATIVE')
    return ToolBase.ensure_file(sdk, 'llvm', 'bin', 'clang++')


class Tool(ToolBase):
    """Wrapper for c++ compiler for NARK."""

    def __init__(self, *args):
        super().__init__(*args)
        try:
            panda_src = self.ensure_dir_env('PANDA_SRC')
            arch = ''
            if Target.HOST == self.target:
                self.cpp = ToolBase.get_cmd_path('c++')
            elif Target.DEVICE == self.target:
                self.cpp = ''
                log.warning('NARK is not supported in open source VMB.')
            elif Target.OHOS == self.target:
                self.cpp = find_ohos_sdk_cpp()
                arch = '--target=aarch64-linux-ohos '
            self.cmd = f'{self.cpp} -I{panda_src} ' + \
                       f'-rdynamic {arch}{self.custom} ' + \
                       '{src} -std=c++17 -shared -o {so} -fpic'
        except RuntimeError as e:
            # don't fail whole run if c++ is unavailable
            log.warning(str(e))
            self.cpp = None

    @property
    def name(self) -> str:
        return 'C++ Compiler for ArkTS Native'

    @property
    def version(self) -> str:
        if not self.cpp:
            return 'not configured'
        return self.sh.run(f'{self.cpp} -v').grep(
            r'.*version\s+([0-9\.]+).*')

    def exec(self, bu: BenchUnit) -> None:
        native_dir = bu.path.joinpath('native')
        if not native_dir.exists():
            return
        # fail test only if native is present
        if not self.cpp:
            raise VmbToolExecError(
                'C++ or "$PANDA_SRC" is not configured!')
        for src in native_dir.glob('*.cpp'):
            so = native_dir.joinpath(f'lib{src.name}').with_suffix('.so')
            res = self.sh.run(
                self.cmd.format(src=src, so=so),
                measure_time=True)
            if res.ret != 0:
                bu.status = BUStatus.COMPILATION_FAILED
                raise VmbToolExecError(f'{self.name} failed: {bu.path}', res)
            bu.result.build.append(
                BuildResult('gcc', self.sh.get_filesize(so), res.tm, res.rss))
            bu.binaries.append(so)

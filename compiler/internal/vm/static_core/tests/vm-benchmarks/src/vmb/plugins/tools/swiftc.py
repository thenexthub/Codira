#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus
from vmb.target import Target

log = logging.getLogger('vmb')


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.swiftc = ToolBase.get_cmd_path('swiftc', 'SWIFTC')
        if Target.HOST != self.target:
            dev_kit = self.ensure_dir_env(f'{"N"}DK_PATH')
            swift_build = self.ensure_dir_env('SWIFT_BUILD')
            self.command = f'{self.swiftc} ' \
                           '-O -gnone -wmo ' \
                           f'-tools-directory {dev_kit}/bin/ ' \
                           '-target aarch64-unknown-linux-and' \
                           f'roid21 -sdk {dev_kit}/sysroot ' \
                           f'-resource-dir {swift_build} {self.custom} ' \
                           '-o {exe} {src}'
        else:
            self.command = f'{self.swiftc} -O -gnone -wmo -v {self.custom} ' \
                           '-o {exe} {src}'

    @property
    def name(self) -> str:
        return 'Swift Compiler'

    @property
    def version(self) -> str:
        return self.sh.run(
            f'{self.swiftc} --version').grep(r'\s*([0-9\.]+)')

    def exec(self, bu: BenchUnit) -> None:
        src = bu.src('.swift')
        exe = src.with_suffix('.exe')
        exe.unlink(missing_ok=True)
        cmd = self.command.format(exe=exe, src=src)
        res = self.sh.run(cmd, measure_time=True, timeout=300)
        if res is None or res.ret != 0 or not exe.is_file():
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed', res)
        exe_size = self.sh.get_filesize(exe)
        bu.result.build.append(
            BuildResult('swiftc', exe_size, res.tm, res.rss))
        bu.binaries.append(exe)

    def kill(self) -> None:
        self.sh.run('pkill swift-driver')
        self.sh.run('pkill swiftc')

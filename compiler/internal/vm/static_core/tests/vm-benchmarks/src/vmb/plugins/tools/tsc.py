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

from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus

log = logging.getLogger('vmb')


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.tsc = ToolBase.get_cmd_path('tsc', 'TSC')

    @property
    def name(self) -> str:
        return 'TypeScript Compiler'

    @property
    def version(self) -> str:
        return self.sh.run(
            f'{self.tsc} --version').grep(r'\s*([0-9\.]+)')

    def exec(self, bu: BenchUnit) -> None:
        """Compile ts -> mjs if any."""
        # libs first
        ts_files = list(bu.libs('.ts')) + \
            [bu.src('.ts')]
        for ts in ts_files:
            if not ts.is_file():
                continue
            res = self.sh.run(
                f'{self.tsc} '
                '--target es2023 '
                '--module es2022 '
                f'{self.custom} '
                f'--outDir {bu.path} {ts}', measure_time=True)
            if self.no_run:
                return
            js = ts.with_suffix('.js')
            if res.ret != 0 or not js.is_file():
                bu.status = BUStatus.COMPILATION_FAILED
                raise VmbToolExecError(f'{self.name} failed', res)
            ToolBase.rename_suffix(js, '.mjs')
            bu.result.build.append(BuildResult('tsc', 0, res.tm, res.rss))

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

from vmb.tool import ToolBase, OptFlags
from vmb.unit import BenchUnit, BUStatus
from vmb.target import Target


class Tool(ToolBase):

    binname = f'd{8}'

    def __init__(self, *args):
        super().__init__(*args)
        opts = self.custom
        if OptFlags.INT in self.flags:
            opts += ' --no-opt --jitless --use-ic --no-expose_wasm '
        if self.target == Target.HOST:
            binpath = ToolBase.get_cmd_path(self.binname, 'V_8')
            self.d_8 = f'{binpath} {opts}'
        elif self.target == Target.DEVICE:
            self.d_8 = f'{self.dev_dir.as_posix()}/v_8/{self.binname} {opts}'
        elif self.target == Target.OHOS:
            self.d_8 = 'LD_LIBRARY_PATH=/data/local/and' \
                      f'roid {self.dev_dir.as_posix()}/v_8/{self.binname} {opts}'
        else:
            raise NotImplementedError(f'Not supported "{self.target}"!')

    @property
    def name(self) -> str:
        return 'V_8 JavaScript Engine'

    @property
    def version(self) -> str:
        return self.x_run(
            f'echo "quit();"|{self.d_8}').grep(r'version\s*([0-9\.]+)')

    def exec(self, bu: BenchUnit) -> None:
        bu_flags, _ = self.get_bu_opts(bu)
        opts = '--max-inlined-bytecode-size=0 ' \
            if OptFlags.DISABLE_INLINING in bu_flags else ''
        mjs = self.x_src(bu, '.mjs')
        res = self.x_run(f'{self.d_8} {opts}{mjs}')
        if self.no_run:
            bu.status = BUStatus.NOT_RUN
            return
        bu.parse_run_output(res)

    def kill(self) -> None:
        self.x_sh.run(f'pkill {self.binname}')

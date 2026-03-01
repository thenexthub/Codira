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

from vmb.tool import ToolBase
from vmb.unit import BenchUnit, BUStatus


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.node = ToolBase.get_cmd_path('node', 'NODE')

    @property
    def name(self) -> str:
        return 'Node'

    @property
    def version(self) -> str:
        return self.sh.run(
            f'{self.node} --version').grep(r'v([0-9\.]+)')

    def exec(self, bu: BenchUnit) -> None:
        mjs = bu.src('.mjs')
        res = self.x_run(
            f'{self.node} {self.custom} {mjs}',
            measure_time=True)
        if self.no_run:
            bu.status = BUStatus.NOT_RUN
            return
        bu.parse_run_output(res)

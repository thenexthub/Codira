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

from vmb.tool import ToolBase
from vmb.unit import BenchUnit
from vmb.target import Target


class Tool(ToolBase):

    def __init__(self, *args) -> None:
        super().__init__(*args)
        libpath = f'LD_LIBRARY_PATH={ToolBase.dev_dir} ' \
                  if self.target in (Target.DEVICE, Target.OHOS) \
                  else ''
        self.cmd = f'{libpath}{{exe}}'

    @property
    def name(self) -> str:
        return 'Run executable'

    def exec(self, bu: BenchUnit) -> None:
        exe = self.x_src(bu, '.exe')
        res = self.x_run(self.cmd.format(exe=exe))
        bu.parse_run_output(res)

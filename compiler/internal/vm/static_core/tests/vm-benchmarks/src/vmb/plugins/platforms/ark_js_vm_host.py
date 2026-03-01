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
from typing import List

from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args, OptFlags

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.es2abc = self.tools_get('es2abc')
        self.ark_js_vm = self.tools_get('ark_js_vm')
        if OptFlags.AOT in self.flags or OptFlags.AOTPGO in self.flags:
            self.aot_compiler = self.tools_get('ark_aot_compiler')
        self.lang = list(self.langs)[0]

    @property
    def name(self) -> str:
        return 'ArkHz on host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def required_tools(self) -> List[str]:
        if OptFlags.AOT in self.flags or OptFlags.AOTPGO in self.flags:
            return ['es2abc', 'ark_js_vm', 'ark_aot_compiler']
        return ['es2abc', 'ark_js_vm']

    @property
    def langs(self) -> List[str]:
        return list(self.args_langs) if self.args_langs else ['ts']

    def run_unit(self, bu: BenchUnit) -> None:
        self.es2abc.exec_lang(bu, lang=self.lang)
        if self.dry_run_stop(bu):
            return
        self.push_unit(bu, '.abc')  # for device; on host does nothing
        if OptFlags.AOT in self.flags or OptFlags.AOTPGO in self.flags:
            self.ark_js_vm.profile(bu)
            self.aot_compiler(bu)
            self.ark_js_vm.profile(bu, with_aot=True)
            self.aot_compiler(bu)
        self.ark_js_vm.exec(bu)

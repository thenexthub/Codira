#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#
#

from vmb.tool import ToolBase
from vmb.unit import BenchUnit


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.panda_root = self.ensure_dir_env('PANDA_BUILD')
        self.ets_vm_opts = '{}'  # no extra options by default, but still valid json
        ark_js = ToolBase.ensure_file(
            self.panda_root, 'bin', 'interop_js', 'ark_js_napi_cli')
        etsstdlib = ToolBase.ensure_file(
            self.panda_root, 'plugins', 'ets', 'etsstdlib.abc')
        self.cmd = f'LD_LIBRARY_PATH={self.panda_root}/lib/interop_js/:{self.panda_root}/lib/ ' \
                   f'ARK_ETS_STDLIB_PATH={etsstdlib} ' \
                   "ETS_VM_OPTS='{ets_vm_opts}' " \
                   'ARK_ETS_INTEROP_JS_GTEST_ABC_PATH={test_zip} ' \
                   'VMB_BENCH_NAME={bench_name} ' \
                   f'{ark_js} ' \
                   '--entry-point={entry_point} ' \
                   '{test_abc}'

    @property
    def name(self) -> str:
        return 'ArkJS VM Interop Host'

    @property
    def version(self) -> str:
        return 'version n/a'

    def exec(self, bu: BenchUnit) -> None:
        raise NotImplementedError

    def run(self, bu: BenchUnit, abc: str, entry_point: str,
            bench_name: str = 'none', zip_path: str = 'none'):
        res = self.sh.run(self.cmd.format(
            test_zip=zip_path,
            test_abc=abc,
            entry_point=entry_point,
            bench_name=bench_name,
            ets_vm_opts=self.ets_vm_opts
        ))
        bu.parse_run_output(res)

    def kill(self) -> None:
        self.sh.run('pkill ark_js_napi_cli')

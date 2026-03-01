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

import logging
from typing import List, Optional
from vmb.cli import Args
from vmb.tool import ToolBase, VmbToolExecError
from vmb.gensettings import GenSettings
from vmb.unit import BenchUnit
from vmb.plugins.platforms.hap_s2s import Platform as HapEtsPlatform

log = logging.getLogger('vmb')


class Platform(HapEtsPlatform):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        ohos_sdk = ToolBase.ensure_dir_env('OHOS_BASE_SDK_HOME')
        self.es2abc_bin = f'{ohos_sdk}/12/ets/build-tools/ets-loader/bin/ark/build/bin/es2abc'
        ToolBase.ensure_file(self.es2abc_bin)

    @property
    def name(self) -> str:
        return 'Application (hap) interop s2d on OHOS'

    @property
    def required_tools(self) -> List[str]:
        return ['es2panda', 'hvigor']

    @property
    def template(self) -> Optional[GenSettings]:
        return GenSettings(src={'.ets'},
                           template='ets-hap.j2',
                           out='.ets',
                           link_to_src=False,
                           link_to_other_src={'.ts', '.js'})

    @property
    def langs(self) -> List[str]:
        return ['ets']

    # pylint: disable=duplicate-code
    def process_dynamic(self, bu: BenchUnit) -> None:
        # process dynamic imports first for es2panda using it
        libs = bu.libs('.ts', '.mjs', '.js')
        if not libs:
            raise RuntimeError('No dynamic imports in ets bench!')
        for lib in libs:
            lib = ToolBase.rename_suffix(lib, '.ts')
            res = self.sh.run(
                f'{self.es2abc_bin} --output={lib.with_suffix(".abc")} --module {lib}')
            if res.ret != 0:
                raise VmbToolExecError('es2abc failed!', res)

    def run_batch(self, bus: List[BenchUnit]) -> None:
        self.run_batch_impl(bus, self.process_dynamic)

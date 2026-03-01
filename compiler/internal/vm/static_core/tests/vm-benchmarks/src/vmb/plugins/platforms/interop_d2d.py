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
from typing import List
from vmb.target import Target
from vmb.unit import BenchUnit, BENCH_PREFIX
from vmb.plugins.platforms.interop_base import InteropPlatformBase

log = logging.getLogger('vmb')


class Platform(InteropPlatformBase):

    @property
    def name(self) -> str:
        return 'Interop ArkJS (d2d) host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def langs(self) -> List[str]:
        """Use ts by default."""
        return self.args_langs if self.args_langs else ['ts']

    def run_unit(self, bu: BenchUnit) -> None:
        libs = bu.libs('.ts', '.js', '.mjs')
        if not libs:
            log.warning('No dynamic imports in interop bench!')
        for lib in libs:
            self.es2abc_interop.run_es2abc(lib, lib.with_suffix('.abc'))
        self.es2abc_interop(bu)
        self.arkjs_interop.run(bu, abc=str(bu.main_bin), entry_point=f'{BENCH_PREFIX}{bu.name}')

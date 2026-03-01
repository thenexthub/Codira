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
from vmb.target import Target
from vmb.unit import BenchUnit, BENCH_PREFIX
from vmb.cli import Args
from vmb.gensettings import GenSettings
from vmb.plugins.platforms.interop_base import InteropPlatformBase

log = logging.getLogger('vmb')


class Platform(InteropPlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        if len(self.args_langs) > 1:
            raise RuntimeError(f'Only one lang for `{self.name}`!')
        self.establish_arkjs_module()

    @property
    def name(self) -> str:
        return 'Interop ArkJS/ArkTS (d2s) host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def langs(self) -> List[str]:
        """Use only one lang."""
        return list(self.args_langs)[:1] if self.args_langs else ['js']

    @property
    def template(self) -> Optional[GenSettings]:
        """Special template for D2S Interop: single lang."""
        lang = self.langs[0]
        ext, tpl = ('.js', 'js.j2') if lang == 'js' else ('.ts', 'ts.j2')
        return GenSettings(src={ext},
                           template=tpl,
                           out=ext,
                           link_to_src=False,
                           link_to_other_src={'.ets'})

    def run_unit(self, bu: BenchUnit) -> None:
        self.es2abc_interop(bu)
        for lib in bu.libs('.ets'):  # compile ETS imports
            abc = lib.with_suffix('.abc')
            if abc.is_file():
                continue
            bu.main_lib = abc
            self.es2panda.run_es2panda(lib, abc, self.es2panda.opts, bu)
            break  # expect only one lib
        if not bu.main_lib.exists():
            raise RuntimeError('No ETS import!')
        zip_path = self.zip_classes(bu, abc=bu.main_lib)
        self.arkjs_interop.run(bu,
                               abc=str(bu.main_bin),
                               entry_point=f'{BENCH_PREFIX}{bu.name}',
                               zip_path=zip_path)

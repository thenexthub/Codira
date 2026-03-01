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

# pylint: disable=duplicate-code
import logging
from typing import List, Optional
from vmb.helpers import copy_file
from vmb.gensettings import GenSettings
from vmb.unit import BenchUnit
from vmb.plugins.platforms.hap_base import HapPlatformBase

log = logging.getLogger('vmb')


class Platform(HapPlatformBase):

    @property
    def name(self) -> str:
        return 'Application (hap) interop d2d on OHOS'

    @property
    def template(self) -> Optional[GenSettings]:
        # Only ts for now
        return GenSettings(src={'.ts'},
                           template='ts-hap.j2',
                           out='.ts',
                           link_to_src=False)

    @property
    def required_tools(self) -> List[str]:
        return ['hvigor']

    @property
    def langs(self) -> List[str]:
        return ['ts']

    def run_batch(self, bus: List[BenchUnit]) -> None:
        hap_unit = self.make_hap_unit()
        module_config_copy = self.make_module_config()
        for bu in bus:
            for lib in bu.libs('.ts'):
                copy_file(lib, self.ability_dir)
            # Create bench ability file
            copy_file(bu.src('.ts', die_on_zero_matches=True),
                      self.ability_dir.joinpath(f'{bu.name}_Ability.ts'))
            # Add bench ability to module.json5
            module_config_copy['module']['abilities'].append(HapPlatformBase.make_ability(bu))
        # flush module.json5
        self.flush_module_config(module_config_copy)
        self.hvigor(hap_unit)

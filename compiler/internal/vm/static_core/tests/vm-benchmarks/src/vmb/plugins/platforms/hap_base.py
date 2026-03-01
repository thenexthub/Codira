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
import os
import shutil
from copy import deepcopy
from pathlib import Path
from typing import Dict, Any
from abc import ABC
from vmb.platform import PlatformBase
from vmb.tool import ToolBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args
from vmb.helpers import copy_file

log = logging.getLogger('vmb')

module_config: Dict[str, Any] = {
    "module": {
        "name": "entry",
        "type": "entry",
        "description": "$string:module_desc",
        "mainElement": "Empty_Ability",
        "deviceTypes": [
          "default",
          "tablet"
        ],
        "deliveryWithInstall": True,
        "installationFree": False,
        "pages": "$profile:main_pages",
        "virtualMachine": "ark",
        "abilities": [
        ]
    }
}

ability_config = {
    "name": "Empty_Ability",
    "srcEntry": "./ets/entryability/Empty_Ability.ts",
    "description": "$string:EntryAbility_desc",
    "icon": "$media:icon",
    "label": "$string:EntryAbility_label",
    "startWindowIcon": "$media:icon",
    "startWindowBackground": "$color:start_window_background",
    "exported": True,
    "skills": [
        {
            "entities": [
              "entity.system.home"
            ],
            "actions": [
              "ohos.want.action.home"
            ]
        }
    ]
}

ohos_dynamic_paths = {
    '@ohos.hilog': {
        'language': 'js',
        'ohmUrl': '@ohos.hilog'},
    "@ohos.app.ability.AbilityConstant": {
        'language': 'js',
        'ohmUrl': '@ohos.app.ability.AbilityConstant'},
    "@ohos.app.ability.UIAbility": {
        'language': 'js',
        'ohmUrl': '@ohos.app.ability.UIAbility'},
    "@ohos.app.ability.Want": {
        'language': 'js',
        'ohmUrl': '@ohos.app.ability.Want'},
    "@ohos.window": {
        'language': 'js',
        'ohmUrl': '@ohos.window'},
}


class HapPlatformBase(PlatformBase, ABC):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.hvigor = self.tools_get('hvigor')
        self.sdk_version = 12  # hardcoded for now
        self.app_name = self.hvigor.app_name
        self.mod_name = self.hvigor.mod_name
        self.resource_dir = Path(
            os.path.realpath(os.path.dirname(__file__))).parent.parent.joinpath('resources', 'hap')
        self.hap_dir = ToolBase.libs.joinpath('hap')
        self.ability_dir = self.hap_dir.joinpath(self.mod_name, 'src', 'main', 'ets', 'entryability')

    @property
    def target(self) -> Target:
        return Target.OHOS

    @staticmethod
    def make_ability(bu: BenchUnit) -> Any:
        bu_ability = deepcopy(ability_config)
        bu_ability['name'] = f'{bu.name}_Ability'
        bu_ability['srcEntry'] = f'./ets/entryability/{bu.name}_Ability.ts'
        return bu_ability

    def make_hap_unit(self) -> BenchUnit:
        shutil.rmtree(self.hap_dir, ignore_errors=True)
        self.hap_dir.mkdir(exist_ok=True, parents=True)
        return BenchUnit(self.hap_dir)

    def make_module_config(self) -> Any:
        # Empty_Ability
        empty_src = self.resource_dir.joinpath('Empty_Ability.ts')
        empty_ts = self.ability_dir.joinpath(empty_src.name)
        copy_file(empty_src, empty_ts)
        # Add Empty_Ability to module.json5
        module_config_copy = deepcopy(module_config)
        module_config_copy['module']['abilities'].append(ability_config)
        return module_config_copy

    def flush_module_config(self, config: Any) -> None:
        module_file = self.hap_dir.joinpath(self.mod_name, 'src', 'main', 'module.json5')
        self.hvigor.emit_config(config, module_file)

    def run_unit(self, bu: BenchUnit) -> None:
        ability_name = f'{bu.name}_Ability'
        self.hdc.run('power-shell wakeup')
        # unlock device
        self.hdc.run('uinput -T -m 1000 1000 1000 100')
        # hilog privacy off
        self.hdc.run('hilog -p off')
        finish_re = '|'.join((f'(Benchmark result: {bu.name})',
                              '(VMB MAIN FINISHED)',
                              '(ETS RUNTIME CREATE ERROR)'))
        res = self.hdc.run_syslog(
            cmd=f'aa start -a {ability_name} -b {self.app_name}',
            finished_marker=finish_re,
            timeout=20)
        self.hdc.run(f'aa force-stop {self.app_name}')
        bu.parse_run_output(res)

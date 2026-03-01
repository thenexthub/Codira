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

import json
import os
import logging
from pathlib import Path
from typing import Union, Any, Dict
from vmb.helpers import create_file, copy_file, load_json
from vmb.tool import ToolBase
from vmb.unit import BenchUnit

here = os.path.realpath(os.path.dirname(__file__))
certs = Path(here).parent.parent.joinpath('resources', 'hap-sign')
log = logging.getLogger('vmb')

hvigor_config = {
    "modelVersion": "5.0.0",
    "dependencies": {},
    "execution": {},
    "logging": {
        "level": "info"
    },
    "debugging": {},
    "nodeOptions": {}
}

build_config: Dict = {
    "app": {
        "signingConfigs": [],
        "products": [
          {
            "name": "default",
            "compileSdkVersion": 12,
            "compatibleSdkVersion": 12,
            "targetSdkVersion": 12,
            "runtimeOS": "OpenHarmony",
          }
        ],
        "buildModeSet": [
          {
            "name": "debug",
          }
        ]
    },
    "modules": [
        {
          "name": "entry",
          "srcPath": "./entry",
          "targets": [
            {
              "name": "default",
              "applyToProducts": [
                "default"
              ]
            }
          ]
        }
    ]
}

build_config_module = {
    "apiType": "stageMode",
    "buildOption": {},
    "targets": [{"name": "default"}]
}

oh_package = {
    "modelVersion": "5.0.0",
    "description": "blah",
    "dependencies": {},
    "devDeppendencies": {}
}

HVIGORFILE_TS = """
import { appTasks } from '@ohos/hvigor-ohos-plugin';
export default {
    system: appTasks, plugins: []
}
"""

MODULE_HVIGORFILE_TS = """
import { hapTasks } from '@ohos/hvigor-ohos-plugin';
export default {
    system: hapTasks, plugins: []
}
"""

app = {
    "app": {
        "bundleName": "com.example.hellopanda",
        "vendor": "example",
        "versionCode": 1000,
        "versionName": "1.0.0",
        "icon": "$media:app_icon",
        "label": "$string:app_name"
    }
}

app_string = {
    "string": [{"name": "app_name", "value": "HelloPanda"}]
}

color = {
    "color": [{
        "name": "start_window_background",
        "value": "#FFFFFF"
    }]
}

string = {
    "string": [
        {"name": "module_desc", "value": "module description"},
        {"name": "EntryAbility_desc", "value": "description"},
        {"name": "EntryAbility_label", "value": "arkUiDemo"}
    ]
}

main_pages = {
    "src": ["pages/Index"]
}


class Tool(ToolBase):

    app_name = 'com.example.hellopanda'
    mod_name = 'entry'

    def __init__(self, *args) -> None:
        super().__init__(*args)
        self.ohos_sdk = self.ensure_dir_env('OHOS_BASE_SDK_HOME')
        self.hvigorw = ToolBase.get_cmd_path('hvigorw', 'HVIGORW')
        self.signing_config = os.environ.get('HAP_SIGNING_CONFIG', '')
        self.resource_dir = Path(here).parent.parent.joinpath('resources', 'hap')

    @property
    def name(self) -> str:
        return 'Build HAP package (hvigorw)'

    @staticmethod
    def emit_config(content: Any, path: Union[str, Path]) -> None:
        p = Path(path)
        p.parent.mkdir(parents=True, exist_ok=True)
        with create_file(p) as f:
            f.write(json.dumps(content, indent=4))

    def exec(self, bu: BenchUnit) -> None:
        if self.signing_config:
            build_config['app']['signingConfigs'] = [{
                "name": "default",
                "material": load_json(self.signing_config)}]
            build_config['app']['products'][0]['signingConfig'] = 'default'
        Tool.emit_config(build_config, bu.path.joinpath('build-profile.json5'))
        Tool.emit_config(oh_package, bu.path.joinpath('oh-package.json5'))
        Tool.emit_config(oh_package, bu.path.joinpath(self.mod_name, 'oh-package.json5'))
        Tool.emit_config(hvigor_config, bu.path.joinpath('hvigor', 'hvigor-config.json5'))
        Tool.emit_config(build_config_module, bu.path.joinpath(self.mod_name, 'build-profile.json5'))
        Tool.emit_config(app, bu.path.joinpath('AppScope', 'app.json5'))
        Tool.emit_config(app_string, bu.path.joinpath(
            'AppScope', 'resources', 'base', 'element', 'string.json'))
        Tool.emit_config(color, bu.path.joinpath(
            self.mod_name, 'src', 'main', 'resources', 'base', 'element', 'color.json'))
        Tool.emit_config(string, bu.path.joinpath(
            self.mod_name, 'src', 'main', 'resources', 'en_US', 'element', 'string.json'))
        Tool.emit_config(string, bu.path.joinpath(
            self.mod_name, 'src', 'main', 'resources', 'base', 'element', 'string.json'))
        Tool.emit_config(main_pages, bu.path.joinpath(
            self.mod_name, 'src', 'main', 'resources', 'base', 'profile', 'main_pages.json'))
        # to do: links instead of files?
        with create_file(bu.path.joinpath('hvigorfile.ts')) as f:
            f.write(HVIGORFILE_TS)
        with create_file(bu.path.joinpath(self.mod_name, 'hvigorfile.ts')) as f:
            f.write(MODULE_HVIGORFILE_TS)
        # icon.png
        icon_src = self.resource_dir.joinpath('icon.png')
        icon = bu.path.joinpath('entry', 'src', 'main', 'resources', 'base', 'media', 'icon.png')
        icon_app = bu.path.joinpath('AppScope', 'resources', 'base', 'media', 'app_icon.png')
        copy_file(icon_src, icon)
        copy_file(icon_src, icon_app)
        # Index.ets
        index_ets_src = self.resource_dir.joinpath('Index.ets')
        index_ets = bu.path.joinpath('entry', 'src', 'main', 'ets', 'pages', 'Index.ets')
        copy_file(index_ets_src, index_ets)
        self.sh.run(f'{self.hvigorw} -d --mode module -p module={self.mod_name}@default '
                    '-p product=default -p debuggable=true --no-daemon assembleHap',
                    cwd=str(bu.path))
        hap = bu.path.joinpath(self.mod_name,
                               f'build/default/outputs/default/{self.mod_name}-default-signed.hap')
        self.ensure_file(hap)
        self.hdc.install(hap, self.app_name)

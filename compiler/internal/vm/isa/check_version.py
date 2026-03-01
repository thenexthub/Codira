#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import os
import yaml

CURRENT_VERSION = "13.0.1.0"

script_dir = os.path.dirname(os.path.realpath(__file__))

isa_file_path = os.path.join(script_dir, 'isa.yaml')


def check_version(yaml_file, cur_version):
    try:
        with open(yaml_file, 'r') as file:
            data = yaml.safe_load(file)
            
            file_version = data.get('version')
            
            if file_version == cur_version:
                print("No change in version!")
                exit(0)
            else:
                print(f"[IMPORTANT] Version change detected ({cur_version} -> {file_version}), please contact the relevant domain.")
                exit(1)
    except Exception as e:
        print(f"Error reading file: {e}")
        exit(1)

check_version(isa_file_path, CURRENT_VERSION)

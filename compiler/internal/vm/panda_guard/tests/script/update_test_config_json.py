#!/usr/bin/env python3
# coding: utf-8

"""
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: update unit test config json field
"""

import argparse
import json
import os

OBFUSCATION_RULES = "obfuscationRules"
APPLY_NAME_CACHE = "applyNameCache"
PRINT_NAME_CACHE = "printNameCache"


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-dir",
                        help="project root directory")
    return parser.parse_args()


def update_json_file(config_path, field_name, field_value):
    fd = os.open(config_path, os.O_RDWR | os.O_CREAT, 0o644)
    with os.fdopen(fd, 'r+', encoding='utf-8') as file:
        data = json.load(file)

        if OBFUSCATION_RULES not in data or not isinstance(data[OBFUSCATION_RULES], dict):
            return

        if field_name in data[OBFUSCATION_RULES]:
            return

        data[OBFUSCATION_RULES][field_name] = field_value
        file.seek(0)
        json.dump(data, file, indent=2, ensure_ascii=False)
        file.truncate()


def modify_json_file(base_path, config_name, cache_name, json_field):
    config_path = os.path.join(base_path, config_name)
    cache_path = os.path.join(base_path, cache_name)
    update_json_file(config_path, json_field, cache_path)


if __name__ == '__main__':
    args = parse_args()
    configsPath = os.path.join(args.project_dir, 'tests/unittest/configs')
    modify_json_file(configsPath, 'context_test_01_config.json', 'context_test_01_name_cache.json', APPLY_NAME_CACHE)
    modify_json_file(configsPath, 'name_cache_test_02_config.json', 'new_name_cache_test.json', PRINT_NAME_CACHE)

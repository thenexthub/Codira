#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import argparse
import json
import os
import re
import shutil
import stat
from pathlib import Path
from typing import Set


def load_json_file(file_path: Path) -> dict:
    """Load JSON file; return parsed data"""
    try:
        with os.fdopen(
            os.open(file_path, os.O_RDONLY, stat.S_IRUSR), "r", encoding="utf-8"
        ) as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error: Hiddable Interface file format is invalid in ({file_path}): {e}")
        return {}


def read_hiddable_apis(std_lib_dir: Path, json_file: Path) -> dict:
    hiddable_apis = {}
    hiddable_api_file = std_lib_dir / json_file

    if not hiddable_api_file.exists():
        print("Error: Hiddable Interface file not found. Will Copy All files.")
        return hiddable_apis

    data = load_json_file(hiddable_api_file)
    for item in data:
        file = item.get("file")
        single_file_apis = item.get("interfaces", [])
        if file:
            hiddable_apis[file] = set(single_file_apis)

    return hiddable_apis


def process_file_with_hiddable_interfaces(
    source_file: Path,
    target_file: Path,
    hiddable_apis: Set[str],
    api_pattern: re.Pattern,
) -> None:
    """Replace export keyword with empty string if interface is hiddable"""
    with os.fdopen(
        os.open(source_file, os.O_RDONLY, stat.S_IRUSR), "r", encoding="utf-8"
    ) as f:
        lines = f.readlines()

    modified_lines = []

    for line in lines:
        match = api_pattern.search(line)
        if match:
            _, api_name = match.groups()
            if api_name in hiddable_apis:
                modified_line = re.sub(r"\bexport \b", "", line)
                modified_lines.append(modified_line)
                continue

        modified_lines.append(line)

    with os.fdopen(
        os.open(
            target_file,
            os.O_RDWR | os.O_CREAT | os.O_TRUNC,
            stat.S_IWUSR | stat.S_IRUSR,
        ),
        "w",
    ) as fout:
        fout.writelines(modified_lines)


def copy_files_with_filter(
    std_lib_dir: Path, target_dir: Path, hiddable_apis: dict
) -> None:
    """Copy stdlib files; filter interfaces by hiddable_APIs.json"""

    api_pattern = re.compile(
        r"^\s*export\s+(?:final\s+|abstract\s+)?(?:native\s+)?(class|interface|@interface|function)\s+(\w+)"
    )

    for root, _, files in os.walk(std_lib_dir):
        relative_path = Path(root).relative_to(std_lib_dir)
        target_sub_dir = target_dir / "std" / relative_path

        target_sub_dir.mkdir(parents=True, exist_ok=True)

        for file in files:
            source_file = Path(root) / file
            relative_file_path = relative_path / file
            relative_file_string = str(relative_file_path)

            if relative_file_string not in hiddable_apis:
                shutil.copy2(source_file, target_sub_dir)
                continue

            hiddable_apis = hiddable_apis[relative_file_string]

            if not hiddable_apis:
                continue
            target_file = target_sub_dir / file
            process_file_with_hiddable_interfaces(
                source_file, target_file, hiddable_apis, api_pattern
            )


def main():
    parser = argparse.ArgumentParser(
        description="Copy and process standard library files with hiddable interfaces filtering."
    )
    parser.add_argument(
        "--std-dir", type=Path, help="Directory containing standard library files"
    )
    parser.add_argument(
        "--escompat-dir", type=Path, help="Directory containing escompact files"
    )
    parser.add_argument(
        "--target-dir", type=Path, help="Target directory for processed files"
    )
    parser.add_argument(
        "--json-file", type=Path, help="The JSON file to read hiddable APIs from"
    )

    args = parser.parse_args()

    std_lib_dir = args.std_dir
    escompat_lib_dir = args.escompat_dir
    target_dir = args.target_dir
    json_file = args.json_file

    if not std_lib_dir or not escompat_lib_dir or not target_dir or not json_file:
        parser.error("std-dir, escompact-dir, target-dir, and json-files are required.")

    target_dir.mkdir(parents=True, exist_ok=True)
    hiddable_apis = read_hiddable_apis(std_lib_dir, json_file)
    copy_files_with_filter(std_lib_dir, target_dir, hiddable_apis)

    escompat_target_dir = target_dir / escompat_lib_dir.name
    shutil.copytree(escompat_lib_dir, escompat_target_dir, dirs_exist_ok=True)


if __name__ == "__main__":
    main()

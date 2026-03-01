#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

import os
import shutil
import stat
import argparse
from pathlib import Path


def should_skip_file(file_path, patterns) -> bool:
    for pattern in patterns:
        if pattern.startswith("*") and file_path.endswith(pattern[1:]):
            return True
        if file_path.endswith(pattern):
            return True
    return False


def copy_with_permissions(src, dst):
    try:
        src_stat = os.stat(src)
        shutil.copy2(src, dst)
        os.chmod(dst, src_stat.st_mode)
    except FileNotFoundError:
        return


def copy_directory_excluding_files(base_dir, output_dir):
    base_path = Path(base_dir)
    output_path = Path(output_dir)
    
    if output_path.exists():
        shutil.rmtree(output_path)
    output_path.mkdir(parents=True)
    
    if base_path.exists():
        base_stat = os.stat(base_path)
        os.chmod(output_path, base_stat.st_mode)
    
    excluded_patterns = [
        "*.taihe.mark",
        "lib/pyrt/lib/python3.11/site-packages/taihe/cli/tryit.py",
        "bin/taihe-tryit",
        "napi_taihe"
    ]
    
    for root, dirs, files in os.walk(base_dir):
        root_path = Path(root)
        relative_path = root_path.relative_to(base_path)

        if "napi_taihe" in str(relative_path):
            dirs[:] = []
            continue

        dest_dir = output_path / relative_path
        
        dest_dir.mkdir(parents=True, exist_ok=True)
        if root_path.exists():
            dir_stat = os.stat(root_path)
            os.chmod(dest_dir, dir_stat.st_mode)
        
        for file in files:
            src_file = root_path / file
            dest_file = dest_dir / file
            file_rel_path = (relative_path / file).as_posix()
            
            if not should_skip_file(file_rel_path, excluded_patterns):
                copy_with_permissions(src_file, dest_file)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Copy directory tree while preserving permissions and excluding specific files"
    )
    parser.add_argument("--input_dir", required=True, help="Source directory to copy")
    parser.add_argument("--output_dir", required=True, help="Destination directory")
    args = parser.parse_args()

    copy_directory_excluding_files(args.input_dir, args.output_dir)

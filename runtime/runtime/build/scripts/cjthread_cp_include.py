#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

import sys
import os
import shutil

def copy_files(txt_file, target_dir):
    # 检查目标目录是否存在，如果不存在则创建
    print(target_dir)
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    with open(txt_file, 'r') as file_list:
        for line in file_list:
            source_file = line.strip()  # 移除行尾的换行符
            path_expanded = os.path.expandvars(source_file)
            absolute_path = os.path.abspath(path_expanded)
            if os.path.isfile(absolute_path):  # 检查文件是否存在
                command = f"rsync -ua {absolute_path} {target_dir}"
                subprocess.run(command)

if __name__ == "__main__":
    txt_file = sys.argv[1]
    target_dir = sys.argv[2]
    os.environ['CPU_FAMILY'] = sys.argv[3]
    os.environ['CPU_CORE'] = sys.argv[4]
    copy_files(txt_file, target_dir)
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Codira Project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

import os
import subprocess
import sys
import shutil
import platform
import argparse

# Check CODIRA_HOME
if not os.environ.get("CODIRA_HOME"):
    print("error: cannot find CODIRA_HOME, please make sure cangjie sdk is configured", file=sys.stderr)
    sys.exit(1)

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))

def check_call(command):
    try:
        return subprocess.check_call(command)
    except subprocess.CalledProcessError as e:
        print(f"Command '{e.cmd}' returned non-zero exit status {e.returncode}.")
        sys.exit(e.returncode) 

def read_code_files(target_dir=None):
    """
    读取指定目录下以.code结尾的文件
    :param target_dir: 目标目录，默认当前目录
    """
    # 默认使用当前工作目录
    if target_dir is None:
        target_dir = os.getcwd()
    
    print(f"正在读取目录 [{target_dir}] 下以.code结尾的文件：")
    print("=" * 50)
    
    # 遍历目录（含子目录）
    for root, dirs, files in os.walk(target_dir):
        for file in files:
            if file.endswith(".code"):
                # 输出文件完整路径
                full_path = os.path.join(root, file)
                print(full_path)
                check_call(['codec', full_path, '--output-type=staticlib'])
                

if __name__ == "__main__":
    # 如需指定目录，修改为 read_code_files("D:/test") 或 "/home/test"
    read_code_files(CURRENT_DIR  + "/expected/c_module")
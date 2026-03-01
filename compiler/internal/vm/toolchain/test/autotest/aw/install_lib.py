#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Python package install.
"""

import subprocess
import sys
from pathlib import Path

python_path = Path(sys.executable)
python_path = (python_path.parent / python_path.stem) if sys.platform == 'win32' else python_path
pip_list = subprocess.check_output([str(python_path), '-m', 'pip', 'list']).decode('utf-8')


def install(pkg_name=None):
    if not pkg_name:  # 按照requirements.txt安装
        install_by_requirements()
    else:  # 安装单个package
        install_by_pkg_name(pkg_name)


def install_by_requirements():
    require_file_path = Path.cwd().parent / 'requirements.txt'
    with open(require_file_path, "r") as require:
        pkgs = require.readlines()
        for pkg in pkgs:
            if pkg in pip_list:
                continue
            install_cmd = [python_path, '-m', 'pip', 'install', pkg.strip(), '-i',
                           'https://cmc-cd-mirror.rnd.huawei.com/pypi/simple/',
                           '--trusted-host', 'cmc-cd-mirror.rnd.huawei.com']
            res = subprocess.call(install_cmd)
            assert res == 0, "Install packages from requirements.txt failed!"


def install_by_pkg_name(pkg_name):
    if pkg_name in pip_list:
        return
    install_cmd = [python_path, '-m', 'pip', 'install', pkg_name, '-i',
                   'http://cmc-cd-mirror.rnd.huawei.com/pypi/simple/',
                   '--trusted-host', 'cmc-cd-mirror.rnd.huawei.com']
    res = subprocess.call(install_cmd)
    assert res == 0, f"Install packages '{pkg_name}' failed!"

#!/usr/bin/env python3
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
from pathlib import Path


def is_sudo_user() -> bool:
    result = int(os.getuid())
    return result == 0


def chown2user(file_or_dir: Path, *, recursive: bool = True) -> None:
    sudo_user = os.getenv("SUDO_USER")
    if sudo_user:
        sudo_group_str = os.getenv("SUDO_GID")
        sudo_group = int(sudo_group_str) if sudo_group_str else 0
        shutil.chown(file_or_dir, sudo_user, sudo_group)
        if file_or_dir.is_dir() and recursive:
            recursive_chown(file_or_dir, sudo_user, sudo_group)


def recursive_chown(file_or_dir: Path, sudo_user: str, sudo_group: int) -> None:
    for dirpath, _, filenames in os.walk(file_or_dir):
        shutil.chown(dirpath, sudo_user, sudo_group)
        for filename in filenames:
            shutil.chown(Path(dirpath).joinpath(filename), sudo_user, sudo_group)

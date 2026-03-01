#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

from dataclasses import dataclass, field
from typing import Set


@dataclass
class GenSettings:
    """Template overrides class.

    In most cases template name, source and bench file extentions
    are set by selected lang,
    but for some platforms these defaults needs to be overriden.
    """

    src: Set[str]  # extensions for source files
    template: str  # template name
    out: str  # extension for generatad file
    link_to_src: bool = False  # softlink from src to bu
    link_to_other_src: Set[str] = field(default_factory=set)  # link for src with other extensions
    print_func: str = ''  # switch b/w console, print, hilog

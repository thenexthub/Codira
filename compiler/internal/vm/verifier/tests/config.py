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

from enum import Enum

JS_FILE = "test_constant_pool_content.js"

API_VERSION_MAP = {
    "API9": "9.0.0.0",
    "API10": "9.0.0.0",
    "API11": "11.0.2.0",
    "API12beta1": "12.0.2.0",
    "API12beta2": "12.0.2.0",
    "API12beta3": "12.0.6.0",
    "API13": "12.0.6.0",
}


# ANSI color codes for terminal output
class Color(Enum):
    WHITE = "\033[37m"
    GREEN = "\033[92m"
    RED = "\033[91m"
    RESET = "\033[0m"

    @staticmethod
    def apply(color, text):
        return f"{color.value}{text}{Color.RESET.value}"
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
#

import os
import logging

import stress_common
from stress_test262 import Test262StressTest
from stress_common import SCRIPT_DIR, TMP_DIR

NODE_GIT_URL = "https://github.com/nodejs/node.git"
NODE_GIT_HASH = "0095726bf3d0a2c01062d98e087526299e709515"


class NodeJSStressTest(Test262StressTest):

    def __init__(self, args):
        super().__init__(args)
        self.js_dir = os.path.join(TMP_DIR, 'abckit_nodejs')

    def get_fail_list_path(self) -> str:
        return os.path.join(SCRIPT_DIR, 'fail_list_node_js.json')

    def prepare(self) -> None:
        if not os.path.exists(self.js_dir):
            stress_common.stress_exec(
                ['git', 'clone', NODE_GIT_URL, self.js_dir], repeats=5)
            stress_common.stress_exec(
                ['git', '-C', self.js_dir, 'checkout', NODE_GIT_HASH])

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
import random
from typing import List
import logging

import stress_common
from stress_test262 import Test262StressTest
from stress_common import SCRIPT_DIR, collect_from

# CC-OFFNXT(WordsTool.66) false positive
HERMES_REVISION = "3feac7b2f9759d83879b04232479041baa805e7b"
HERMES_URL = "https://github.com/facebook/hermes.git"


class HermesStressTest(Test262StressTest):

    def __init__(self, args):
        super().__init__(args)
        self.js_dir = os.path.join(stress_common.TMP_DIR, 'abckit_hermes')

    def get_fail_list_path(self) -> str:
        return os.path.join(SCRIPT_DIR, 'fail_list_hermes.json')

    def prepare(self) -> None:
        self.download_hermes()

    def download_hermes(self) -> None:
        if not os.path.exists(self.js_dir):
            stress_common.stress_exec(
                ['git', 'clone', HERMES_URL, self.js_dir], repeats=5)
            stress_common.stress_exec(
                ['git', '-C', self.js_dir, 'checkout', HERMES_REVISION])

    def collect(self) -> List[str]:
        logger = stress_common.create_logger()
        tests: List[str] = []
        sp = os.path.join(self.js_dir, 'test')
        tests.extend(
            collect_from(
                sp, lambda name: name.endswith('.js') and not name.startswith(
                    '.')))
        random.shuffle(tests)

        logger.debug('Total tests: %s', len(tests))
        return tests

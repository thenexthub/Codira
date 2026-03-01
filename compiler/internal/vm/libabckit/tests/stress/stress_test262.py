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

from typing import List, Tuple, Final, Dict

import stress_common

from stress_test import StressTest, Test, Result
from stress_common import SCRIPT_DIR, TMP_DIR, collect_from

TEST262_GIT_URL = "https://gitee.com/hufeng20/test262.git"
TEST262_GIT_HASH = "6f4601d095a3899d6102f2c320b671495cbe8757"

EXCLUDED_TESTS = [
    'regress-4595.js',
    'module-jit-reachability.js',
]


class Test262StressTest(StressTest):

    def __init__(self, args):
        rc_lib_path: Final[str] = f'{args.build_dir}/arkcompiler/runtime_core'
        ets_rc_lib_path: Final[
            str] = f'{args.build_dir}/arkcompiler/ets_runtime'
        icu_lib_path: Final[str] = f'{args.build_dir}/thirdparty/icu/'
        zlib_lib_path: Final[str] = f'{args.build_dir}/thirdparty/zlib/'
        self.runtime_env: Dict[str, str] = {
            'LD_LIBRARY_PATH':
            f'{rc_lib_path}:{ets_rc_lib_path}:{icu_lib_path}:{zlib_lib_path}'
        }

        self.js_dir = os.path.join(TMP_DIR, 'abckit_test262')
        self.cp = self.get_compiler_path(args.build_dir)
        self.jvm = os.path.join(args.build_dir,
                                'arkcompiler/ets_runtime/ark_js_vm')
        self.timeout = 10
        self.repeats = 1
        self.build_dir = args.build_dir

    def get_fail_list_path(self) -> str:
        return os.path.join(SCRIPT_DIR, 'fail_list_test262.json')

    def run_single(self, test: Test) -> Result:
        result = stress_common.run_abckit(self.build_dir, test.source,
                                          test.abc, '/tmp/tmp.abc')
        if result.returncode != 0:
            error = stress_common.parse_stdout(str(result.returncode),
                                               result.stdout)
            return Result(test.source, error)
        return Result(test.source, "0")

    def compile_single(self, src: str) -> Tuple[str, str, int]:
        logger = stress_common.create_logger()
        abc_path = src + '.abc'
        abc_path = abc_path.replace("[", "_").replace("]", "_")
        cmd = [self.cp, '--module', '--merge-abc', '--output', abc_path, src]
        result = stress_common.stress_exec(cmd,
                                           allow_error=True,
                                           print_command=True)
        if result.returncode == 0 and not os.path.exists(abc_path):
            logger.debug(
                'WARNING: for %s es2abc has %s return code, but has no output',
                src, result.returncode)
        return src, abc_path, result.returncode

    def prepare(self) -> None:
        self.download_262()

    def get_compiler_path(self, build_dir) -> str:
        return os.path.join(build_dir, 'arkcompiler/ets_frontend/es2abc')

    def download_262(self) -> None:
        if not os.path.exists(self.js_dir):
            stress_common.stress_exec(
                ['git', 'clone', TEST262_GIT_URL, self.js_dir], repeats=5)
            stress_common.stress_exec(
                ['git', '-C', self.js_dir, 'checkout', TEST262_GIT_HASH])

    def collect(self) -> List[str]:
        logger = stress_common.create_logger()
        tests: List[str] = []
        tests.extend(
            collect_from(
                self.js_dir, lambda name: name.endswith('.js') and not name.
                startswith('.')))
        random.shuffle(tests)

        logger.debug('Total tests: %s', len(tests))
        for excluded in EXCLUDED_TESTS:
            tests = list(filter(lambda name: excluded not in name, tests))
        logger.debug('Tests after exclude: %s', len(tests))

        return tests

    def run_js_test_single(self, test: Test, repeats: int = 1) -> Result:
        entry: str = f'--entry={os.path.basename(test.source)}'
        cmd = [self.jvm, entry, test.abc]
        result = stress_common.stress_exec(cmd,
                                           allow_error=True,
                                           print_command=True,
                                           env=self.runtime_env,
                                           timeout=self.timeout,
                                           print_output=False,
                                           repeats=repeats)
        if result.returncode == 0:
            return Result(test.source, 0)

        return Result(test.source, result.returncode, result.stdout,
                      result.stderr)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
import multiprocessing.pool
import os
import sys
from distutils.dir_util import copy_tree
import random
from typing import Tuple, Dict, Final, List
import logging

import stress_test
import stress_common
from stress_test import StressTest, Test, Result

from stress_common import SCRIPT_DIR, TMP_DIR, collect_from

TEST_ETS_SOURCES_ROOT = os.path.join(SCRIPT_DIR, '..', '..', '..',
                                     'static_core', 'plugins', "ets", "tests",
                                     'ets_func_tests')
STDLIB_ETS_SOURCES_ROOT = os.path.join(SCRIPT_DIR, '..', '..', '..',
                                       'static_core', 'plugins', "ets",
                                       "stdlib")


def get_stdlib_path(build_dir: str) -> str:
    return os.path.join(
        build_dir,
        'gen/arkcompiler/runtime_core/static_core/plugins/ets/etsstdlib.abc')


def get_verifier_path(build_dir: str) -> str:
    return os.path.abspath(
        os.path.join(build_dir, f'arkcompiler/runtime_core/verifier'))


def get_config_path(build_dir: str) -> str:
    return f'{build_dir}/arkcompiler/ets_frontend/arktsconfig.json'


def stress_single(test: Test, out_dir) -> Result:
    stress_abc = test.abc + '.stress.abc'
    result = stress_common.run_abckit(out_dir, test.source, test.abc,
                                      stress_abc)

    if result.returncode == 0:
        return Result(test.source, 0)
    return Result(test.source, result.returncode, result.stdout, result.stderr)


class StsStressTest(StressTest):

    def __init__(self, args):
        super().__init__()
        self.ets_dir = os.path.join(TMP_DIR, 'abckit_test_ets')
        self.stdlib_dir = os.path.join(TMP_DIR, 'abckit_stdlib')

        self.cp = self.get_compiler_path(args.build_dir)
        self.vp = get_verifier_path(args.build_dir)
        self.sp = get_stdlib_path(args.build_dir)
        self.config = get_config_path(args.build_dir)
        self.build_dir = args.build_dir

        rc_lib_path: Final[str] = f'{args.build_dir}/arkcompiler/runtime_core'

        self.cenv: Dict[str, str] = {'LD_LIBRARY_PATH': rc_lib_path}
        self.venv: Dict[str, str] = {
            'LD_LIBRARY_PATH':
            f'{rc_lib_path}:{args.build_dir}/thirdparty/icu/'
        }

    def get_fail_list_path(self) -> str:
        return os.path.join(SCRIPT_DIR, 'fail_list_sts.json')

    def run_single(self, test: Test) -> Result:
        stress_result: Result = stress_single(test, self.build_dir)
        verify_result_one: Result = self.verify_single(test)

        r2p = Test(test.source, test.abc + ".stress.abc")
        verify_result_two: Result = self.verify_single(r2p)

        if stress_result.result != 0:
            error = stress_common.parse_stdout(stress_result.result,
                                               stress_result.stdout)
            return Result(test.source, error)
        # Stress test passed

        if verify_result_two.result == 0:
            return Result(test.source, "0")

        if verify_result_one.result != verify_result_two.result:
            msg = f'Verifier result changed. Was {verify_result_one.result}, now {verify_result_two.result}'
            return Result(test.source, msg)

        return Result(test.source, "0")

    def prepare(self) -> None:
        self.download_ets()
        self.download_stdlib()

    def download_ets(self) -> None:
        logger = stress_common.create_logger()
        abs_path = os.path.abspath(TEST_ETS_SOURCES_ROOT)
        logger.debug('Ets test download from %s', abs_path)
        if not os.path.exists(abs_path):
            logger.debug('Not exists %s', abs_path)
            return
        copy_tree(abs_path, self.ets_dir)

    def download_stdlib(self) -> None:
        logger = stress_common.create_logger()

        abs_path = os.path.abspath(STDLIB_ETS_SOURCES_ROOT)
        logger.debug('Stdlib download from %s', abs_path)
        if not os.path.exists(abs_path):
            logger.debug('Not exists %s', abs_path)
            return
        copy_tree(abs_path, self.stdlib_dir)

    def get_compiler_path(self, build_dir) -> str:
        return os.path.abspath(
            os.path.join(build_dir, 'arkcompiler/ets_frontend/es2panda'))

    def compile_single(self, src: str) -> Tuple[str, str, int]:
        logger = stress_common.create_logger()
        abc_path = src + '.abc'
        cmd = [
            self.cp, '--ets-module', '--arktsconfig', self.config,
            f'--output={abc_path}', src
        ]
        result = stress_common.stress_exec(cmd,
                                           allow_error=True,
                                           print_command=True,
                                           env=self.cenv)
        if result.returncode == 0 and not os.path.exists(abc_path):
            logger.debug(
                'WARNING: for %s es2abc has %s return code, but has no output',
                src, result.returncode)
        return src, abc_path, result.returncode

    def verify_single(self, test: Test) -> Result:
        boot_panda_files = f'--boot-panda-files={self.sp}'
        cmd = [self.vp, boot_panda_files, '--load-runtimes=ets', test.abc]
        result = stress_common.stress_exec(cmd,
                                           allow_error=True,
                                           print_command=True,
                                           env=self.venv)
        return Result(test.source, result.returncode)

    def collect(self) -> List[str]:
        logger = stress_common.create_logger()
        tests: List[str] = []
        tests.extend(collect_from(self.ets_dir, lambda name: name.endswith('.ets') and not name.startswith('.')))
        tests.extend(collect_from(self.stdlib_dir, lambda name: name.endswith('.ets') and not name.startswith('.')))

        random.shuffle(tests)
        logger.debug('Total tests: %s', len(tests))
        return tests

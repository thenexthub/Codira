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

import multiprocessing.pool
import sys
from abc import abstractmethod
from typing import Tuple, List

import stress_common


class Test:

    def __init__(self, source: str, abc: str):
        self.source = source
        self.abc = abc


class Result:

    def __init__(self, source, result, stdout=None, stderr=None):
        self.source = source
        self.result = result
        self.stdout = stdout
        self.stderr = stderr


class StressTest:

    def run(self, tests: List[Test]):
        logger = stress_common.create_logger()
        logger.debug('Running ABCKit...')
        result_all = {}
        counter = 0

        with multiprocessing.pool.ThreadPool(stress_common.NPROC) as pool:
            for result in pool.imap(self.run_single, tests):
                result_all[result.source] = {}
                result_all[result.source]['error'] = result.result
                counter += 1

        return result_all

    @abstractmethod
    def run_single(self, test: Test) -> Result:
        pass

    @abstractmethod
    def compile_single(self, src: str) -> Tuple[str, str, int]:
        pass

    @abstractmethod
    def prepare(self) -> None:
        pass

    def build(self) -> List[Test]:
        logger = stress_common.create_logger()
        tests: List[str] = self.collect()

        logger.debug('Running compiler...')
        compiled_tests: List[Test] = []
        counter = 0
        with multiprocessing.pool.ThreadPool(stress_common.NPROC) as pool:
            for js_path, abc_path, retcode in pool.imap(self.compile_single,
                                                        tests,
                                                        chunksize=20):
                if retcode == 0:
                    compiled_tests.append(Test(js_path, abc_path))
                counter += 1

        logger.debug('Tests successfully compiled: %s', len(compiled_tests))
        return compiled_tests

    @abstractmethod
    def collect(self) -> List[str]:
        pass

    @abstractmethod
    def get_compiler_path(self, build_dir) -> str:
        pass

    @abstractmethod
    def get_fail_list_path(self, build_dir) -> str:
        pass

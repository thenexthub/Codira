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

from stress_sts_with_runtime import StsStressTestWithRuntime
from stress_hermes_with_runtime import HermesStressTestWithRuntime
from stress_test262_with_runtime import Test262StressTestWithRuntime
from stress_hermes import HermesStressTest
from stress_node_js import NodeJSStressTest
from stress_sts import StsStressTest
from stress_test262 import Test262StressTest
import sys
from typing import List

import stress_common

from stress_test import Test


def main():
    logger = stress_common.create_logger()

    args = stress_common.get_args()

    logger.debug('Initializing stress tests...')
    logger.debug(f'Runner args: {args}')

    stress_tests = [
        Test262StressTest(args),
        StsStressTest(args),
        NodeJSStressTest(args),
        HermesStressTest(args),
    ]

    if args.with_runtime:
        stress_tests.append(Test262StressTestWithRuntime(args.repeats, args))
        stress_tests.append(HermesStressTestWithRuntime(args))
        stress_tests.append(StsStressTestWithRuntime(args.ark_path, args))

    logger.debug('Running stress tests...')

    for test in stress_tests:
        logger.debug(f'Running {test.__class__.__name__} test suite...')

        test.prepare()

        tests: List[Test] = test.build()

        results = test.run(tests)

        fail_list = stress_common.get_fail_list(results)

        if args.update_fail_list:
            stress_common.update_fail_list(test.get_fail_list_path(),
                                           fail_list)
            logger.debug('Failures/Total: %s/%s', len(fail_list), len(results))
            return 0

        if not stress_common.check_regression_errors(test.get_fail_list_path(),
                                                     fail_list):
            return 1

        if not stress_common.check_fail_list(test.get_fail_list_path(),
                                             fail_list):
            logger.debug('Failures/Total: %s/%s', len(fail_list), len(results))

        logger.debug('No regressions')
        logger.debug('Failures/Total: %s/%s', len(fail_list), len(results))

    return 0


if __name__ == '__main__':
    sys.exit(main())

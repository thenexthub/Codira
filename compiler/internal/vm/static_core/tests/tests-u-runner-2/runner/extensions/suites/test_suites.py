#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from runner.enum_types.params import TestEnv
from runner.extensions.suites.test_suite_registry import ITestSuite, suite_registry
from runner.suites.test_suite import GTestSuite, TestSuite


@suite_registry.register("ets")
def make_test_suite(test_env: TestEnv) -> ITestSuite:
    return TestSuite(test_env)


@suite_registry.register("gtest")
def make_gtest_suite(test_env: TestEnv) -> ITestSuite:
    return GTestSuite(test_env)

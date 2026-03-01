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

from pathlib import Path

from runner.enum_types.params import TestEnv
from runner.extensions.flows.test_flow_registry import ITestFlow, workflow_registry
from runner.options.options import IOptions
from runner.suites.gtest_flow import GTestFlow
from runner.suites.test_standard_flow import TestStandardFlow


@workflow_registry.register("ets")
def make_test_standard_flow(test_env: TestEnv, test_path: Path, *, params: IOptions, test_id: str) -> ITestFlow:
    return TestStandardFlow(test_env, test_path, params=params, test_id=test_id)


@workflow_registry.register("gtest")
def make_gtest_flow(test_env: TestEnv, test_path: Path, *, params: IOptions, test_id: str) -> ITestFlow:
    precompiled_tests = True
    return GTestFlow(test_env, test_path, params=params, test_id=test_id, precompiled_tests=precompiled_tests)

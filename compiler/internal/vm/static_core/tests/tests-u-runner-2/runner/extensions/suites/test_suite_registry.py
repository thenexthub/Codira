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

from abc import ABC, abstractmethod
from pathlib import Path

from runner.extensions.flows.test_flow_registry import ConfigRegistry, ITestFlow


class ITestSuite(ABC):
    excluded: int
    excluded_tests: list[Path]

    @property
    @abstractmethod
    def list_root(self) -> Path:
        ...

    @abstractmethod
    def process(self, force_generate: bool) -> list[ITestFlow]:
        ...


suite_registry: ConfigRegistry[ITestSuite] = ConfigRegistry()

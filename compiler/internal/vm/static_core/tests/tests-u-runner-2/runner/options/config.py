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

import re
from typing import Any

from runner.logger import Log
from runner.options.cli_args_wrapper import CliArgsWrapper
from runner.options.options import IOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.options_workflow import WorkflowOptions

_LOGGER = Log.get_logger(__file__)


class Config(IOptions):
    __CONFIG_TYPE = 'type'

    def __init__(self, args: dict[str, Any]):       # type: ignore[explicit-any]
        super().__init__(None)
        CliArgsWrapper.setup(args)
        self.general = GeneralOptions(args, self)
        self.test_suite = TestSuiteOptions(args, self.general)
        self.workflow = WorkflowOptions(args, self.test_suite)

    def __str__(self) -> str:
        return self._to_str(indent=0)

    def get_command_line(self) -> str:
        options = ' '.join([
            self.general.get_command_line(),
        ])
        options_str = re.sub(r'\s+', ' ', options, flags=re.IGNORECASE | re.DOTALL)

        return options_str

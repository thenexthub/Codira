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

_PLUGINS = [
    "ark_config",
    "compiler",
    "compiler_verification.expression_verifier",
    "debug",
    "dev_log",
    "disassembler",
    "expect",
    "internal_tests",
    "logs",
    "rich_logging",
    "runtime",
    "timeout",
    "walker",
    "mirrors._plugin",
    "trio_tracer",
]
pytest_plugins = [f"{__package__}.{p}" for p in _PLUGINS]

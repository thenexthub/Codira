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

from typing import List

import pytest
from pytest import Config, Item, Parser

INTERNAL_TESTS = "internal_tests"


def config_internal_tests(config: Config) -> bool:
    ret = config.getoption(INTERNAL_TESTS)
    if ret is None:
        ret = config.getini(INTERNAL_TESTS)
    if ret is not None:
        return bool(ret)
    return False


def pytest_addoption(parser: Parser) -> None:
    """Add options to control log capturing."""
    group = parser.getgroup("arkdb")

    def add_option_ini(*option, dest, default=None, opt_type=None, **kwargs) -> None:
        parser.addini(dest, default=default, type=opt_type, help="Default value for " + option[0])
        group.addoption(*option, dest=dest, **kwargs)

    add_option_ini(
        "--internal-tests",
        "-I",
        dest=INTERNAL_TESTS,
        default=False,
        opt_type="bool",
        help=("Run internals tests."),
    )


def pytest_collection_modifyitems(items: List[Item], config: Config) -> None:
    if config_internal_tests(config):
        return
    internal_test = pytest.mark.skip(reason="need internal_tests=True option to run")
    for item in items:
        if "internal_test" in item.keywords:
            item.add_marker(internal_test)


def pytest_configure(config: Config) -> None:
    config.addinivalue_line("markers", "internal_test: mark internal tests")

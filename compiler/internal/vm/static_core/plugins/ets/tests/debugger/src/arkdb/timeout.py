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

import logging
from functools import wraps
from inspect import iscoroutinefunction
from typing import List

import pytest
import trio
from pytest import Config, Function, Item, Parser, hookimpl

pytest_plugins = ["pytest_trio"]

DEFAULT_TIMEOUT: float = 300.0
LOG = logging.getLogger(__name__)


def timeout_decorator(seconds: float = DEFAULT_TIMEOUT):

    def decorator(func):
        @wraps(func)
        async def wrapped(*args, **kwds):
            LOG.debug("Fail after %s seconds", seconds)
            with trio.fail_after(seconds):
                return await func(*args, **kwds)

        return wrapped

    return decorator


def config_timeout(config: Config) -> float:
    ret = config.getoption("timeout")
    if ret is None:
        ret = config.getini("timeout")
    if ret is not None:
        return float(ret)
    return DEFAULT_TIMEOUT


def pytest_addoption(parser: Parser) -> None:
    """Add options to control log capturing."""
    group = parser.getgroup("arkdb")

    def _add_option_ini(option, dest, default=None, opt_type=None, **kwargs):
        parser.addini(dest, default=default, type=opt_type, help="Default value for " + option)
        group.addoption(option, dest=dest, **kwargs)

    _add_option_ini(
        "--timeout",
        dest="timeout",
        default=DEFAULT_TIMEOUT,
        opt_type="string",
        help=("Timeout for each test."),
    )


@hookimpl(trylast=True)
def pytest_collection_modifyitems(items: List[Item], config: Config) -> None:
    seconds = config_timeout(config)
    for item in items:
        if isinstance(item, Function):
            test_func = item.obj
            if iscoroutinefunction(test_func) and item.get_closest_marker("timeout") is None:
                item.add_marker(pytest.mark.timeout(seconds=seconds))


@hookimpl(hookwrapper=True)
def pytest_runtest_call(item: Item):
    if isinstance(item, Function):
        marker = item.get_closest_marker("timeout")
        if marker:
            if item.get_closest_marker("trio") is None:
                raise RuntimeError("timeout marker can only be used by Trio tests")
            default_seconds = config_timeout(item.config)
            seconds = marker.kwargs.get("seconds", default_seconds)
            item.obj = timeout_decorator(seconds=seconds)(item.obj)
    yield


def pytest_configure(config: Config):
    # So that it shows up in 'pytest --markers' output:
    config.addinivalue_line(
        "markers",
        f"timeout(seconds={config_timeout(config)}):"
        " mark the trio test with timeout; it will be run in timeout context",
    )

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
import os
from pathlib import Path

from pytest import FixtureRequest, Parser, fixture

from .ark_config import str2bool
from .logs import ARK_ERR, ARK_OUT


def dev_log_filters():
    from .mirrors.type_cache import LOG as cache_log
    from .runtime import LOG as runtime_log

    return {
        None: logging.INFO,
        "trio-websocket": logging.WARNING,
        "trio_cdp": logging.DEBUG,
        runtime_log.name: min(ARK_ERR, ARK_OUT),
        cache_log.name: logging.DEBUG,
    }


DEV_LOG_FILTERS = dev_log_filters()


@fixture(scope="session", autouse=True)
def dev_log(request: FixtureRequest):
    opt = request.config.getoption("dev_log", request.config.getini("dev_log"), skip=True)
    if opt:
        levels = DEV_LOG_FILTERS
        for name, level in levels.items():
            logger = logging.getLogger(name)
            if logger.level:
                logger.setLevel(min(level, logger.level))
            else:
                logger.setLevel(level)


def pytest_addoption(parser: Parser):

    # --ark-build-path
    parser.addini(
        name="ark_build_path",
        default=os.environ.get("ARK_BUILD", "."),
        type="paths",
        help="Default value for --ark-build-path",
    )
    parser.getgroup("run").addoption(
        "--ark-build-path",
        dest="ark_build_path",
        type=Path,
        help="Ark compiler build directory. Default: $ARK_BUILD or .",
    )

    # --dev-log
    parser.addini(
        name="dev_log",
        default=False,
        type="bool",
        help="Default value for --dev-log",
    )
    parser.getgroup("dev").addoption(
        "--dev-log",
        dest="dev_log",
        help="Enable dev log level filter",
        type=str2bool,
    )

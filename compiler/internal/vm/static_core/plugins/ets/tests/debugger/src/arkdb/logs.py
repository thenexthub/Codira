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
import re
from typing import Any

from pytest import FixtureRequest, fixture

ARK_OUT, ARK_OUT_NAME = (logging.DEBUG + 5, "ARK_OUT")
ARK_ERR, ARK_ERR_NAME = (logging.DEBUG + 6, "ARK_ERR")
TRIO, TRIO_NAME = (logging.CRITICAL + 1, "TRIO_TASK")

logging.addLevelName(ARK_OUT, ARK_OUT_NAME)
logging.addLevelName(ARK_ERR, ARK_ERR_NAME)
logging.addLevelName(TRIO, TRIO_NAME)


class RichLogger(logging.LoggerAdapter):

    def process(self, msg: Any, kwargs):
        extra = kwargs.pop("extra", dict())
        r = kwargs.pop("rich", None)
        if r:
            extra["rich"] = r
        return msg, {**kwargs, "extra": extra}


def logger(name: str) -> RichLogger:
    return RichLogger(logging.getLogger(name))


def _loggername_from_nodeid(nodeid: str) -> str:
    if (pos := nodeid.find("[")) != -1:
        nodeid = nodeid[:pos]
    return re.sub(r"/|::", ".", nodeid)


@fixture
def log(request: FixtureRequest) -> RichLogger:
    return RichLogger(logging.getLogger(_loggername_from_nodeid(request.node.nodeid)))

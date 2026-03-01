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

from time import sleep

import trio
from pytest import mark

pytestmark = mark.internal_test


@mark.timeout(seconds=0.01)
@mark.xfail(raises=trio.TooSlowError, reason="Timeout test")
async def test_timeout_fail():
    await trio.sleep(0.02)


@mark.timeout(seconds=0.02)
async def test_timeout_pass():
    await trio.sleep(0.01)


@mark.timeout(seconds=0.02)
@mark.xfail(raises=RuntimeError, reason="Timeout test")
def test_timeout():
    sleep(1)

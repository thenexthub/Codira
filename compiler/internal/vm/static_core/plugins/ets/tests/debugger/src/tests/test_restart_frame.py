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

from arkdb.walker import BreakpointWalkerType


TEST_RESTART_FRAME_CODE = """\
function foo(): int {
    undefined          // #BP{FOO_BRK}
    let bRes = boo();
    return bRes + 100;
}

function boo(): int {
    undefined          // #BP{BOO_BRK}
    let gRes = goo();
    return gRes + 200;
}

function goo(): int {
    undefined          // #BP{GOO_BRK}
    return 300;
}

function main(): void {
    foo();
}\
"""


async def test_restart_frame(
    breakpoint_walker: BreakpointWalkerType,
):
    async with breakpoint_walker(TEST_RESTART_FRAME_CODE) as walker:
        stop_point = await anext(walker)
        assert stop_point.hit_breakpoint_labels().pop() == "FOO_BRK"

        # restart top frame (function foo)
        paused = await stop_point.client.restart_frame(0)  # now we returned into frame of the main
        assert paused.call_frames[0].function_name == "main"

        stop_point = await anext(walker)
        assert stop_point.hit_breakpoint_labels().pop() == "FOO_BRK"
        stop_point = await anext(walker)
        assert stop_point.hit_breakpoint_labels().pop() == "BOO_BRK"

        # restart top frame (function boo)
        paused = await stop_point.client.restart_frame(0)
        assert paused.call_frames[0].function_name == "foo"

        stop_point = await anext(walker)
        assert stop_point.hit_breakpoint_labels().pop() == "BOO_BRK"
        stop_point = await anext(walker)
        assert stop_point.hit_breakpoint_labels().pop() == "GOO_BRK"

        # restart top frame (function goo)
        paused = await stop_point.client.restart_frame(0)
        assert paused.call_frames[0].function_name == "boo"

        stop_point = await anext(walker)
        assert stop_point.hit_breakpoint_labels().pop() == "GOO_BRK"

        # restart two frames on top
        paused = await stop_point.client.restart_frame(1)
        assert paused.call_frames[0].function_name == "foo"

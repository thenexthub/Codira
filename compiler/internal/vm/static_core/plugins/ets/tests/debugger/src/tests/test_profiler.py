#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#
#

from signal import SIGTERM
from typing import Final
from arkdb.compiler import StringCodeCompiler
from arkdb.debug_client import DebugLocator
from arkdb.runtime import Runtime
import trio

CODE: Final[
    str
] = """\
function foo1(repetitions: int) {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
}
function foo2(repetitions: int) {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
}
function foo3(size: int) {
    let num: double = 0.0;
    let signNum: int = 1;
    for (let firstIndex: int = 0; firstIndex < size; firstIndex++) {
        num += signNum / (2.0 * firstIndex + 1);
        signNum *= -1;
    }
    foo1(size);
    for (let secondIndex: int = 0; secondIndex < size; secondIndex++) {
        num += signNum / (2.0 * secondIndex + 1);
        signNum *= -1;
    }
}
function foo() {
    let repetitions: int = 5;
    while (true) {
        foo1(repetitions);
        foo2(repetitions);
        foo3(repetitions);
    }
}
function main(): int {
    foo();
    return 0;
}
"""


async def test_profiler(
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
) -> None:
    script_file = code_compiler.compile(CODE)
    async with ark_runtime.run(nursery, module=script_file, profile=True) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            await client.resume()
            await client.profiler_enable()
            await client.profiler_set_sampling_interval(200)
            await client.profiler_start()
            await trio.sleep(0.1)
            await client.profiler_stop()
            await client.profiler_disable()

        process.terminate()
    assert process.returncode == -SIGTERM

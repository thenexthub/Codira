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

import os
import warnings

from pathlib import Path
from typing import Final

import trio

from arkdb.compiler import StringCodeCompiler
from arkdb.debug_client import DebugLocator
from arkdb.extensions import debugger as ext_debugger
from arkdb.runtime import Runtime, RuntimeProcess, Options
from arkdb.source_meta import parse_source_meta
from arkdb.logs import RichLogger


IFACE: Final[
    str
] = """\
export interface AdditionalInterface {
    foo(): void
}
"""

ADDITIONAL: Final[
    str
] = """\
import { AdditionalInterface } from "./iface"

export class AdditionalClass implements AdditionalInterface {
    foo(): void {
        let a = 0;
        a += 1; // #BP{10}
        let b = 1;
        b += 1;
    }
}
"""

CODE: Final[
    str
] = """\
import { AdditionalInterface } from "./iface"

function main(): int {
    let a = 0;
    a += 1; // #BP{1}
    let pro = new StdProcess.ProcessManager();
    const abcFile = pro.getEnvironmentVar("DEBUGGER_DYNAMIC_LOAD_FILE");
    const abcFiles = abcFile.split(":");
    const linker = new AbcRuntimeLinker(undefined, abcFiles);
    a += 1; // #BP{3}
    const cls1 = linker.loadClass("additional.AdditionalClass", true);
    arktest.assertNE(cls1, undefined);
    const elem = cls1.createInstance() as AdditionalInterface;
    elem.foo();
    let b = 1;
    b += 1; // #BP{4}
    return 0;
}
"""


class RuntimeExitStatusWarning(UserWarning):
    pass


def check_exit_status(process: RuntimeProcess, log: RichLogger):
    status = process.returncode
    if status != 0:
        msg = f"Runtime exit status is {status}"
        log.warning(msg)
        warnings.warn(msg, RuntimeExitStatusWarning)


async def test_dynamic_breakpoints_resolve(
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
    log: RichLogger,
) -> None:
    iface_file = code_compiler.compile(IFACE, name="iface")
    script_file = code_compiler.compile(CODE)
    additional_file = code_compiler.compile(ADDITIONAL, name="additional")
    os.environ["DEBUGGER_DYNAMIC_LOAD_FILE"] = str(additional_file.panda_file.resolve())
    iface_path = [Path(iface_file.panda_file.resolve())]

    meta1 = parse_source_meta(CODE)
    meta2 = parse_source_meta(ADDITIONAL)

    additional_options = Options(boot_panda_files=iface_path)

    async with ark_runtime.run(nursery, module=script_file, additional_options=additional_options) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()

            meta_breakpoints_lines1 = set(bp.line_number for bp in meta1.breakpoints)
            meta_breakpoints_lines2 = set(bp.line_number for bp in meta2.breakpoints)

            set_breakpoints1 = await client.get_possible_and_set_breakpoint_by_url(
                [
                    ext_debugger.UrlBreakpointRequest(line_number, url=script_file.source_file.name)
                    for line_number in meta_breakpoints_lines1
                ]
            )
            set_breakpoints2 = await client.get_possible_and_set_breakpoint_by_url(
                [
                    ext_debugger.UrlBreakpointRequest(line_number, url=additional_file.source_file.name)
                    for line_number in meta_breakpoints_lines2
                ]
            )
            assert meta_breakpoints_lines1 == set(bp.line_number for bp in set_breakpoints1)
            assert meta_breakpoints_lines2 == set(bp.line_number for bp in set_breakpoints2)

            paused = await client.resume_and_wait_for_paused()
            assert bool(paused.hit_breakpoints)
            assert paused.call_frames[0].location.line_number == 4
            assert paused.call_frames[0].function_name == "main"

            paused1 = await client.resume_and_wait_for_paused()
            assert bool(paused1.hit_breakpoints)
            assert paused1.call_frames[0].location.line_number == 9
            assert paused1.call_frames[0].function_name == "main"

            await client.resume_and_wait_for_paused()

            paused3 = await client.resume_and_wait_for_paused()
            assert bool(paused3.hit_breakpoints)
            assert paused3.call_frames[0].location.line_number == 5
            assert paused3.call_frames[0].function_name == "foo"

            paused4 = await client.resume_and_wait_for_paused()
            assert bool(paused4.hit_breakpoints)
            assert paused4.call_frames[0].location.line_number == 15
            assert paused4.call_frames[0].function_name == "main"

            await client.resume()

        check_exit_status(process, log)

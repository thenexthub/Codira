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

from pathlib import Path
from signal import SIGTERM

from cdp import runtime, debugger
from pytest import mark, param
import trio

from arkdb.compiler import Compiler, ark_compile
from arkdb.debug_client import DebugLocator
from arkdb.runnable_module import ScriptFile
from arkdb.runtime import Runtime
from arkdb.source_meta import parse_source_meta


@mark.parametrize(
    "test_file",
    [param(Path(__file__).with_suffix(".ets"))],
)
async def test_execution_without_pauses(
    tmp_path: Path,
    ark_compiler: Compiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
    test_file: Path,
) -> None:
    script_file: ScriptFile = ark_compile(
        test_file,
        tmp_path,
        ark_compiler,
    )
    meta = parse_source_meta(test_file)
    assert len(meta.breakpoints) >= 1
    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect_as_no_pauses(nursery) as client:
            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()
            # Continue until return from `main`
            await client.continue_to_location(
                script_id=runtime.ScriptId("1"), line_number=meta.breakpoints[0].line_number
            )
        process.terminate()
    assert process.returncode == -SIGTERM


@mark.parametrize(
    "test_file",
    [param(Path(__file__).parent.joinpath("test_coroutines.ets"))],
)
async def test_coroutines(
    tmp_path: Path,
    ark_compiler: Compiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
    test_file: Path,
) -> None:
    async with ark_runtime.run(nursery, module=ark_compile(test_file, tmp_path, ark_compiler)) as process:
        meta = parse_source_meta(test_file)
        assert len(meta.breakpoints) >= 1

        async with debug_locator.connect_as_no_pauses(nursery) as client:
            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()
            await client.resume()
            await trio.sleep(1)
            session_count = await client.get_session_count()
            assert session_count >= 2

            try:
                # set and trigger breakpoint
                session_abc, session_def = await _test_sessions_breakpoint(client, meta)
                if session_abc is None or session_def is None:
                    return

                # continue to location
                line_number_def = await _test_continue_to_location(client, session_def)

                # stepinto/stepout/stepover
                await _test_sessions_stepping(client, session_abc, session_def, meta, line_number_def)

                # resume and pause
                await _test_resume_and_pause(client, session_abc, session_def)

            finally:
                process.terminate()


async def _test_sessions_breakpoint(client, meta):
    await client.add_breakpoint_for_all_sessions(
        debugger.Location(script_id=runtime.ScriptId("1"), line_number=meta.breakpoints[0].line_number)
    )

    sessions_paused_events = await client.sessions_wait_for(debugger.Paused, wait_time=2.0)
    if len(sessions_paused_events) != 2:
        return None, None

    session_id_abc, paused_abc = sessions_paused_events[0]
    session_abc = await client.get_session(session_id_abc)
    assert session_abc is not None
    assert paused_abc.call_frames[0].location.line_number == meta.breakpoints[0].line_number

    session_id_def, _ = sessions_paused_events[1]
    session_def = await client.get_session(session_id_def)
    assert session_def is not None

    return session_abc, session_def


async def _trigger_session_send(session_connection, cmd):
    await session_connection.execute(cmd)


async def _test_continue_to_location(client, session_def):
    sessions_paused_events = await client.sessions_wait_for(
        event_type=debugger.Paused,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(
            session_def.connection,
            debugger.continue_to_location(location=debugger.Location(script_id=runtime.ScriptId("1"), line_number=17)),
        ),
    )
    assert len(sessions_paused_events) == 1
    session_id, paused = sessions_paused_events[0]
    assert session_id == session_def.session_id
    line_number_def = paused.call_frames[0].location.line_number
    assert line_number_def == 17
    return line_number_def


async def _test_sessions_stepping(client, session_abc, session_def, meta, line_number_def):
    sessions_paused_events = await client.sessions_wait_for(
        event_type=debugger.Paused,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(session_abc.connection, debugger.step_into()),
    )
    assert len(sessions_paused_events) == 1
    session_id, paused = sessions_paused_events[0]
    assert session_id == session_abc.session_id
    assert paused.call_frames[0].location.line_number == 27

    # session_abc step_out
    sessions_paused_events = await client.sessions_wait_for(
        event_type=debugger.Paused,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(session_abc.connection, debugger.step_out()),
    )
    assert len(sessions_paused_events) == 1
    session_id, paused = sessions_paused_events[0]
    assert session_id == session_abc.session_id
    assert paused.call_frames[0].location.line_number == meta.breakpoints[0].line_number + 1

    # session_abc step_over
    sessions_paused_events = await client.sessions_wait_for(
        event_type=debugger.Paused,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(session_abc.connection, debugger.step_over()),
    )
    assert len(sessions_paused_events) == 1
    session_id, paused = sessions_paused_events[0]
    assert session_id == session_abc.session_id
    assert paused.call_frames[0].location.line_number == meta.breakpoints[0].line_number + 2

    # session_def step_over
    sessions_paused_events = await client.sessions_wait_for(
        event_type=debugger.Paused,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(session_def.connection, debugger.step_over()),
    )
    assert len(sessions_paused_events) == 1
    session_id, paused = sessions_paused_events[0]
    assert session_id == session_def.session_id
    assert paused.call_frames[0].location.line_number == line_number_def + 1


async def _test_resume_and_pause(client, session_abc, session_def):
    # send resume to session_abc and wait all sessions resumed
    events = await client.sessions_wait_for(
        event_type=debugger.Resumed,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(session_abc.connection, debugger.resume()),
    )
    assert len(events) == 2
    assert {event[0] for event in events} == {session_abc.session_id, session_def.session_id}

    # send pause to session_def and wait all sessions paused
    events = await client.sessions_wait_for(
        event_type=debugger.Paused,
        wait_time=1.0,
        trigger=lambda: _trigger_session_send(session_def.connection, debugger.pause()),
    )
    assert len(events) == 2
    assert {event[0] for event in events} == {session_abc.session_id, session_def.session_id}

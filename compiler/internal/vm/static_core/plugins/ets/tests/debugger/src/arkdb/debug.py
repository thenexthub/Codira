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

from collections.abc import AsyncIterator
from contextlib import AbstractAsyncContextManager, asynccontextmanager
from dataclasses import dataclass
from signal import SIGTERM
from typing import Protocol

import trio
from pytest import fixture

from arkdb import layouts
from arkdb.compiler import StringCodeCompiler
from arkdb.debug_client import DebuggerClient, DebugLocator
from arkdb.debug_types import Locator, Paused, paused_locator
from arkdb.logs import RichLogger, logger
from arkdb.runnable_module import ScriptFile
from arkdb.runtime import DEFAULT_ENTRY_POINT, Runtime, RuntimeProcess
from arkdb.source_meta import SourceMeta, parse_source_meta

LOG = logger(__name__)


class CompileAndRunType(Protocol):
    def __call__(
        self,
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
    ) -> AbstractAsyncContextManager[RuntimeProcess]:
        pass


@fixture
def compile_and_run(
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
) -> CompileAndRunType:
    """
    Return a :class:`CompileAndRunType` function that compiles and executes ``code_or_file`` script.
    """

    @asynccontextmanager
    async def run(
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
    ) -> AsyncIterator[RuntimeProcess]:
        script_file = code_compiler.compile(source_code=code_or_file) if isinstance(code_or_file, str) else code_or_file
        meta = parse_source_meta(script_file.read_text())
        script_file.log(LOG, highlight_lines=[bp.line_number for bp in meta.breakpoints])
        async with ark_runtime.run(
            nursery,
            module=script_file,
            entry_point=entry_point,
            debug=False,
        ) as process:
            yield process
            await process.wait()

    return run


@dataclass
class DebugContext:
    script_file: ScriptFile
    client: DebuggerClient
    process: RuntimeProcess
    meta: SourceMeta


class CompileAndResumeType(Protocol):
    def __call__(
        self,
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
    ) -> AbstractAsyncContextManager[DebugContext]:
        pass


@fixture
def debug_locator(code_compiler: StringCodeCompiler) -> DebugLocator:
    """
    Return a :class:`DebugLocator` instance.
    """
    return DebugLocator(code_compiler, url="ws://localhost:19015")


@asynccontextmanager
async def _connect_and_set_breakpoints(
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
    meta: SourceMeta,
    script_file: ScriptFile,
    process: RuntimeProcess,
):
    try:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()
            for bp in meta.breakpoints:
                bp.breakpoint_id, bp.locations = await client.set_breakpoint_by_url(
                    url=script_file.source_file.name,
                    line_number=bp.line_number,
                )
            yield DebugContext(
                script_file=script_file,
                client=client,
                process=process,
                meta=meta,
            )
            await trio.lowlevel.checkpoint()
    finally:
        process.terminate()
        with trio.CancelScope(deadline=1, shield=True):
            await process.wait()


@fixture
def compile_and_resume(
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
) -> CompileAndResumeType:

    @asynccontextmanager
    async def run(
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
    ) -> AsyncIterator[DebugContext]:
        script_file = code_compiler.compile(source_code=code_or_file) if isinstance(code_or_file, str) else code_or_file
        meta = parse_source_meta(script_file.read_text())
        script_file.log(LOG, highlight_lines=[bp.line_number for bp in meta.breakpoints])
        async with ark_runtime.run(
            nursery,
            module=script_file,
            entry_point=entry_point,
            debug=True,
        ) as process:
            async with _connect_and_set_breakpoints(nursery, debug_locator, meta, script_file, process) as context:
                yield context
        if process.returncode != -SIGTERM:
            raise RuntimeError()

    return run


class StopOnPausedType(Protocol):
    def __call__(
        self,
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = ...,
    ) -> AbstractAsyncContextManager[Paused]:
        pass


@fixture
def run_and_stop_on_breakpoint(
    compile_and_resume: CompileAndResumeType,
    log: RichLogger,
) -> StopOnPausedType:
    """
    Return a :class:`StopOnPausedType` function that return :class:`debug_types.Paused` context.
    """

    @asynccontextmanager
    async def run(
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
    ) -> AsyncIterator[Paused]:
        async with compile_and_resume(code_or_file, entry_point=entry_point) as context:
            paused = await context.client.resume_and_wait_for_paused()
            log.info(
                context.script_file.source_file,
                rich=await layouts.paused_layout(
                    paused=paused_locator(paused=paused, client=context.client, meta=context.meta),
                    url=context.script_file.source_file,
                ),
            )
            yield Paused(locator=Locator(client=context.client), data=paused)

    return run

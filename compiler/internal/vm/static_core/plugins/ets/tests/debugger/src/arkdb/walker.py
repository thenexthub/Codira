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

from collections.abc import AsyncIterator, Sequence
from contextlib import AbstractAsyncContextManager, asynccontextmanager
from typing import Protocol

import trio
from cdp import debugger
from pytest import fixture
from rich.console import RenderableType
from typing_extensions import Self

from arkdb import layouts
from arkdb.debug import DEFAULT_ENTRY_POINT, CompileAndResumeType, DebugContext
from arkdb.debug_client import DebuggerClient
from arkdb.debug_types import Locator, Paused
from arkdb.logs import logger
from arkdb.runnable_module import ScriptFile
from arkdb.runtime import ProcessCapture, RuntimeProcess
from arkdb.source_meta import SourceMeta

LOG = logger(__name__)


class PausedStep(Paused):
    def __init__(self, locator: Locator, data: debugger.Paused, capture: ProcessCapture) -> None:
        super().__init__(locator, data)
        self.capture = capture


class BreakpointWalker(AsyncIterator[PausedStep]):
    def __init__(
        self,
        client: DebuggerClient,
        meta: SourceMeta,
        process: RuntimeProcess,
        script_file: ScriptFile,
        sync_capture_timeout: float,
    ) -> None:
        self.client = client
        self.meta = meta
        self.process = process
        self.script_file = script_file
        self.sync_capture_timeout = sync_capture_timeout
        self._break = False
        self.paused: Paused | None = None

    def __aiter__(self) -> AsyncIterator[PausedStep]:
        return self

    async def __anext__(self) -> PausedStep:
        self.paused = None
        if self._break:
            raise StopAsyncIteration
        capture = self.process.capture
        nout = len(capture.stdout)
        nerr = len(capture.stderr)
        paused = await self.client.resume_and_wait_for_paused()
        with trio.move_on_after(self.sync_capture_timeout):
            await self.process.sync_capture()
        self.paused = PausedStep(
            locator=Locator(client=self.client, meta=self.meta),
            data=paused,
            capture=ProcessCapture(
                stdout=capture.stdout[nout:],
                stderr=capture.stderr[nerr:],
            ),
        )
        return self.paused

    @property
    def capture(self):
        return self.process.capture

    @classmethod
    def from_context(cls, context: DebugContext, sync_capture_timeout: float) -> Self:
        return cls(
            client=context.client,
            meta=context.meta,
            process=context.process,
            script_file=context.script_file,
            sync_capture_timeout=sync_capture_timeout,
        )

    def stop(self) -> None:
        self._break = True

    async def layout(self, skip_scopes: Sequence[str] = ("global")) -> RenderableType:
        if self.paused is None:
            await trio.lowlevel.checkpoint()
            return "[red]Not paused[/red]"
        return await layouts.paused_layout(
            paused=self.paused,
            url=self.script_file.source_file,
            skip_scopes=skip_scopes,
        )

    async def log_layout(self, msg, *args, skip_scopes: Sequence[str] = ("global"), **kwargs):
        LOG.info(msg, *args, **kwargs, rich=await self.layout(skip_scopes))


class BreakpointWalkerType(Protocol):
    def __call__(
        self,
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
    ) -> AbstractAsyncContextManager[BreakpointWalker]:
        pass


@fixture
def sync_capture_timeout() -> float:
    return 0.1


@fixture
def breakpoint_walker(
    compile_and_resume: CompileAndResumeType,
    sync_capture_timeout: float,
) -> BreakpointWalkerType:
    """
    Return :class:`BreakpointWalkerType`.

    Creates a fixture to run the code in debug mode.

        >>> async with breakpoint_walker(code) as walker:
        >>>     async for paused in walker:
        ...         vars: Dict[str, object] = await paused.frame(0).scope("local").meta_variables()

    Returns an asynchronous generator that calls Resume at each increment and returns Paused when stopped.
    """

    @asynccontextmanager
    async def run(
        code_or_file: str | ScriptFile,
        *,
        entry_point: str = DEFAULT_ENTRY_POINT,
        sync_capture_timeout: float = sync_capture_timeout,
    ) -> AsyncIterator[BreakpointWalker]:
        async with compile_and_resume(code_or_file, entry_point=entry_point) as context:
            yield BreakpointWalker.from_context(
                context=context,
                sync_capture_timeout=sync_capture_timeout,
            )

    return run

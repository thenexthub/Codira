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

from collections import defaultdict
from contextlib import asynccontextmanager
import typing
from typing import Any, AsyncGenerator, Callable, Coroutine, Dict, Generator, Iterator, Optional, TypeAlias

import cdp
import cdp.util
import trio
import trio_cdp
from cdp import debugger, runtime, target

T: TypeAlias = trio_cdp.T
E = typing.TypeVar("E")
T_JSON_DICT: TypeAlias = cdp.util.T_JSON_DICT
BUFF_SIZE: int = 10


class Proxy(typing.Generic[E]):
    def __init__(self) -> None:
        self._value: Optional[E] = None

    @property
    def value(self) -> E:
        if self._value is None:
            raise ValueError()
        return self._value

    @value.setter
    def value(self, value: E):
        self._value = value


class ArkConnection:

    def __init__(self, conn: trio_cdp.CdpBase, nursery: trio.Nursery) -> None:
        self._conn = conn
        self._nursery = nursery
        self.context_id: runtime.ExecutionContextId

    @property
    def connection(self) -> trio_cdp.CdpBase:
        return self._conn

    @property
    def nursery(self) -> trio.Nursery:
        return self._nursery

    @property
    def session_id(self) -> target.SessionID | None:
        return self._conn.session_id

    def listen(self, *event_types, buffer_size: int = BUFF_SIZE) -> trio.MemoryReceiveChannel:
        return self._conn.listen(*event_types, buffer_size=buffer_size)

    async def open_session(self, target_id: target.TargetID) -> trio_cdp.CdpSession | None:
        if isinstance(self._conn, trio_cdp.CdpConnection):
            conn: trio_cdp.CdpConnection = self._conn
            return await conn.open_session(target_id)
        return None  # noqa: ASYNC910

    async def send(self, cmd: Generator[dict, T, Any]) -> T:
        return await self._conn.execute(cmd)

    async def send_and_wait_for(
        self,
        cmd: Generator[dict, T, Any],
        event_type: typing.Type[E],
        buffer_size=BUFF_SIZE,
    ) -> E:
        async with self.wait_for(event_type, buffer_size) as proxy:
            await self._conn.execute(cmd)
        return proxy.value

    @asynccontextmanager
    async def wait_for(self, event_type: typing.Type[T], buffer_size=BUFF_SIZE) -> AsyncGenerator[Proxy[T], None]:
        cmd_proxy: trio_cdp.CmEventProxy
        proxy = Proxy[T]()
        async with self._conn.wait_for(event_type, buffer_size) as cmd_proxy:
            yield proxy
        proxy.value = cmd_proxy.value  # type: ignore[attr-defined]


class DebugConnection(trio_cdp.CdpConnection):

    async def aclose(self):
        await super().aclose()
        self._close_channels()

    async def reader_task(self):
        try:
            await super()._reader_task()
        finally:
            self._close_channels()

    def _close_channels(self):
        for channels_dict in self._get_all_channels():
            channels = set([ch for s in channels_dict.values() for ch in s])
            channels_dict.clear()
            for ch in channels:
                ch.close()

    def _get_all_channels(self) -> Iterator[defaultdict[T, set[trio.MemorySendChannel]]]:
        yield self.channels
        for session in self.sessions.values():
            yield session.channels


async def connect_cdp(nursery: trio.Nursery, url, max_retries: int) -> DebugConnection:
    counter = max_retries
    while True:
        try:
            conn = DebugConnection(
                await trio_cdp.connect_websocket_url(
                    nursery,
                    url,
                    max_message_size=trio_cdp.MAX_WS_MESSAGE_SIZE,
                )
            )
            nursery.start_soon(conn.reader_task)
            return conn
        except OSError:
            counter -= 1
            if counter == 0:
                raise
            await trio.sleep(1)


class ScriptsCache:
    def __init__(self) -> None:
        self._lock = trio.Lock()
        self._scripts: Dict[runtime.ScriptId, debugger.ScriptParsed] = {}

    async def __getitem__(self, script_id: runtime.ScriptId) -> debugger.ScriptParsed:
        async with self._lock:
            if script_id not in self._scripts:
                raise KeyError(script_id)
            return self._scripts[script_id]

    async def get(self, script_id: runtime.ScriptId) -> Optional[debugger.ScriptParsed]:
        async with self._lock:
            return self._scripts.get(script_id)

    async def get_by_url(self, url: str) -> Optional[debugger.ScriptParsed]:
        async with self._lock:
            for s in self._scripts.values():
                if url == s.url:
                    return s
            return None

    async def add(self, *scripts: debugger.ScriptParsed) -> None:
        async with self._lock:
            for s in scripts:
                self._scripts[s.script_id] = s


class SourcesCache:

    def __init__(
        self,
    ) -> None:
        self._lock = trio.Lock()
        self._scripts: Dict[runtime.ScriptId, str] = {}

    async def get(
        self,
        script_id: runtime.ScriptId,
        get_source: Optional[Callable[[runtime.ScriptId], Coroutine[Any, Any, str]]] = None,
    ) -> str:
        async with self._lock:
            result = self._scripts.get(script_id)
            if result is None:
                if get_source is not None:
                    result = await get_source(script_id)
                    self._scripts[script_id] = result
                else:
                    raise IndexError()
            return result

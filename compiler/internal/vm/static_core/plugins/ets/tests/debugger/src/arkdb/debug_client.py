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

from contextlib import asynccontextmanager
import dataclasses
from inspect import getfullargspec
from typing import Any, AsyncIterator, Callable, Dict, Iterator, List, Literal, Optional, Tuple, Type, TypeAlias
from typing import Awaitable

import trio
import trio_cdp
from cdp import debugger, profiler, runtime, target

from arkdb.compiler import StringCodeCompiler
from arkdb.debug_connection import ArkConnection, Proxy, ScriptsCache, SourcesCache, connect_cdp
from arkdb.extensions import debugger as ext_debugger
from arkdb.extensions import profiler as ext_profiler

T: TypeAlias = trio_cdp.T
BUFF_SIZE: int = 10


@dataclasses.dataclass
class DebuggerConfig:
    pause_on_exceptions_mode: Literal["none", "caught", "uncaught", "all"] = "none"


class DebuggerClient:
    def __init__(
        self,
        connection: ArkConnection,
        config: DebuggerConfig,
        debugger_id: runtime.UniqueDebuggerId,
        scripts: ScriptsCache,
        sources: SourcesCache,
        context: runtime.ExecutionContextDescription,
        code_compiler: StringCodeCompiler,
    ) -> None:
        self.connection = connection
        self.config = config
        self.debugger_id = debugger_id
        self.scripts = scripts
        self.sources = sources
        self.context = context
        self.code_compiler = code_compiler
        # Sessions management, which corresponds to ArkTS coroutines
        self._sessions: dict[target.SessionID, ArkConnection] = {}
        self._sessions_lock = trio.Lock()

    @staticmethod
    async def _resume_and_wait_for_paused(connection: ArkConnection) -> debugger.Paused:
        async with DebuggerClient._wait_for(connection, debugger.Paused) as proxy:
            await connection.send_and_wait_for(debugger.resume(), debugger.Resumed)
        await trio.lowlevel.checkpoint()
        return proxy.value

    @staticmethod
    async def _run_if_waiting_for_debugger(connection: ArkConnection) -> debugger.Paused:
        return await connection.send_and_wait_for(
            runtime.run_if_waiting_for_debugger(),
            debugger.Paused,
        )

    @staticmethod
    @asynccontextmanager
    async def _wait_for(connection: ArkConnection, event_type: Type[T], buffer_size=BUFF_SIZE):
        proxy: Proxy[T]
        async with connection.wait_for(event_type=event_type, buffer_size=buffer_size) as proxy:
            yield proxy

    async def get_session_count(self) -> int:
        async with self._sessions_lock:
            return len(self._sessions)

    async def get_session(self, session_id: target.SessionID) -> ArkConnection | None:
        async with self._sessions_lock:
            return self._sessions.get(session_id)

    async def sessions_wait_for(
        self,
        event_type: Type[T],
        wait_time: Optional[float] = None,
        trigger: Optional[Callable[[], Awaitable[None]]] = None,
    ) -> List[Tuple[target.SessionID, T]]:
        events_received: List[Tuple[target.SessionID, T]] = []
        receivers = await self._setup_receivers(event_type)

        try:
            if trigger is not None:
                await trigger()

            if wait_time is None:
                await self._collect_first_events(receivers, events_received)
            else:
                await self._collect_events_with_timeout(receivers, events_received, wait_time)
        finally:
            pass

        return events_received

    async def configure(self, nursery: trio.Nursery):
        self._listen(
            nursery,
            [
                self._create_on_execution_contexts_cleared(nursery),
                self._create_on_attached_to_target(nursery),
                self._create_on_detached_from_target(nursery),
            ]
            + self._create_additional_events_handlers(),  # noqa W503
        )
        await self.set_pause_on_exceptions()

    async def run_if_waiting_for_debugger(self) -> debugger.Paused:
        return await DebuggerClient._run_if_waiting_for_debugger(self.connection)

    async def set_pause_on_exceptions(
        self,
        mode: Optional[Literal["none", "caught", "uncaught", "all"]] = None,
    ):
        await self.connection.send(
            debugger.set_pause_on_exceptions(
                mode if mode is not None else self.config.pause_on_exceptions_mode,
            ),
        )

    async def resume(self) -> debugger.Resumed:
        return await self.connection.send_and_wait_for(
            debugger.resume(),
            debugger.Resumed,
        )

    async def resume_and_wait_for_paused(self) -> debugger.Paused:
        return await DebuggerClient._resume_and_wait_for_paused(self.connection)

    async def send_and_wait_for_paused(self, send_arg) -> debugger.Paused:
        async with DebuggerClient._wait_for(self.connection, debugger.Paused) as proxy:
            await self.connection.send(send_arg)
        await trio.lowlevel.checkpoint()
        return proxy.value

    async def continue_to_location(
        self,
        script_id: runtime.ScriptId,
        line_number: int,
    ) -> debugger.Paused:
        return await self.connection.send_and_wait_for(
            debugger.continue_to_location(
                location=debugger.Location(
                    script_id=script_id,
                    line_number=line_number,
                ),
            ),
            debugger.Paused,
        )

    async def get_script_source(
        self,
        script_id: runtime.ScriptId,
    ) -> str:
        return await self.connection.send(debugger.get_script_source(script_id))

    async def get_script_source_cached(
        self,
        script_id: runtime.ScriptId,
    ) -> str:
        return await self.sources.get(script_id, self.get_script_source)

    async def get_properties(
        self,
        object_id: runtime.RemoteObjectId,
        own_properties: Optional[bool] = None,
        accessor_properties_only: Optional[bool] = None,
        generate_preview: Optional[bool] = None,
    ) -> Tuple[
        List[runtime.PropertyDescriptor],
        Optional[List[runtime.InternalPropertyDescriptor]],
        Optional[List[runtime.PrivatePropertyDescriptor]],
        Optional[runtime.ExceptionDetails],
    ]:
        return await self.connection.send(
            runtime.get_properties(
                object_id=object_id,
                own_properties=own_properties,
                accessor_properties_only=accessor_properties_only,
                generate_preview=generate_preview,
            )
        )

    async def set_breakpoint(
        self,
        location: debugger.Location,
        condition: Optional[str] = None,
    ) -> Tuple[debugger.BreakpointId, debugger.Location]:
        return await self.connection.send(
            debugger.set_breakpoint(
                location=location,
                condition=condition,
            ),
        )

    async def remove_breakpoint(self, breakpoint_id: debugger.BreakpointId) -> None:
        return await self.connection.send(
            debugger.remove_breakpoint(
                breakpoint_id=breakpoint_id,
            ),
        )

    async def add_breakpoint_for_all_sessions(
        self,
        location: debugger.Location,
        condition: Optional[str] = None,
    ) -> Tuple[debugger.BreakpointId, debugger.Location]:
        async with self._sessions_lock:
            for connection in self._sessions.values():
                await connection.send(
                    debugger.set_breakpoint(
                        location=location,
                        condition=condition,
                    )
                )

        return await self.connection.send(
            debugger.set_breakpoint(
                location=location,
                condition=condition,
            ),
        )

    async def remove_breakpoints_by_url(self, url: str) -> None:
        await self.connection.send(ext_debugger.remove_breakpoints_by_url(url))

    async def set_breakpoint_by_url(self, *args, **kwargs) -> Tuple[debugger.BreakpointId, List[debugger.Location]]:
        return await self.connection.send(debugger.set_breakpoint_by_url(*args, **kwargs))

    async def get_possible_breakpoints(
        self,
        start: debugger.Location,
        end: Optional[debugger.Location] = None,
        restrict_to_function: Optional[bool] = None,
    ) -> List[debugger.BreakLocation]:
        return await self.connection.send(
            debugger.get_possible_breakpoints(
                start=start,
                end=end,
                restrict_to_function=restrict_to_function,
            )
        )

    async def get_possible_and_set_breakpoint_by_url(
        self,
        locations: List[ext_debugger.UrlBreakpointRequest],
    ) -> List[ext_debugger.CustomUrlBreakpointResponse]:
        return await self.connection.send(ext_debugger.get_possible_and_set_breakpoint_by_url(locations))

    async def set_breakpoints_active(self, active: bool) -> None:
        await self.connection.send(debugger.set_breakpoints_active(active=active))

    async def set_skip_all_pauses(self, skip: bool) -> None:
        await self.connection.send(debugger.set_skip_all_pauses(skip=skip))

    async def evaluate_on_call_frame(
        self,
        call_frame_id: debugger.CallFrameId,
        expression: str,
        object_group: Optional[str] = None,
        include_command_line_api: Optional[bool] = None,
        silent: Optional[bool] = None,
        return_by_value: Optional[bool] = None,
    ) -> Tuple[runtime.RemoteObject, Optional[runtime.ExceptionDetails]]:
        return await self.connection.send(
            debugger.evaluate_on_call_frame(
                call_frame_id=call_frame_id,
                expression=expression,
                object_group=object_group,
                include_command_line_api=include_command_line_api,
                silent=silent,
                return_by_value=return_by_value,
            )
        )

    async def evaluate(self, expression: str) -> tuple[runtime.RemoteObject, runtime.ExceptionDetails | None]:
        return await self.connection.send(runtime.evaluate(expression))

    async def restart_frame(self, frame_number: int) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.restart_frame(debugger.CallFrameId(str(frame_number))))

    async def step_into(self) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.step_into())

    async def step_out(self) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.step_out())

    async def step_over(self) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.step_over())

    async def profiler_enable(self) -> None:
        await self.connection.send(profiler.enable())

    async def profiler_set_sampling_interval(self, interval: int) -> None:
        await self.connection.send(profiler.set_sampling_interval(interval))

    async def profiler_start(self) -> None:
        await self.connection.send(profiler.start())

    async def profiler_stop(self) -> ext_profiler.ProfileArray:
        return await self.connection.send(ext_profiler.profiler_stop())

    async def profiler_disable(self) -> None:
        await self.connection.send(profiler.disable())

    def _configure_session_events(self, _: ArkConnection) -> None:
        return

    def _create_additional_events_handlers(self) -> list:
        return []

    def _create_on_attached_to_target(self, nursery: trio.Nursery):
        def _spawn_on_attached_to_target(event: target.AttachedToTarget):
            async def _on_attached_to_target():  # noqa: ASYNC910
                try:
                    session = await self.connection.open_session(event.target_info.target_id)
                    if session is None:
                        return
                    await trio.lowlevel.checkpoint()

                    connection = ArkConnection(session, nursery)
                    async with self._sessions_lock:
                        self._sessions[session.session_id] = connection
                    self._configure_session_events(connection)
                    await connection.send(runtime.run_if_waiting_for_debugger())
                except trio_cdp.BrowserError as exc:
                    # `Target.attachToTarget` is OK to fail when session has already detached
                    if exc.code != -32602:
                        raise
                except trio.EndOfChannel:
                    pass

            nursery.start_soon(_on_attached_to_target)

        return _spawn_on_attached_to_target

    def _create_on_detached_from_target(self, nursery: trio.Nursery):
        def _spawn_on_detached_from_target(event: target.DetachedFromTarget):
            async def _on_detached_from_target():  # noqa: ASYNC910
                session_id = event.session_id
                async with self._sessions_lock:
                    if session_id in self._sessions:
                        del self._sessions[session_id]

            nursery.start_soon(_on_detached_from_target)

        return _spawn_on_detached_from_target

    def _create_on_execution_contexts_cleared(self, nursery: trio.Nursery):
        def _on_execution_contexts_cleared(_: runtime.ExecutionContextsCleared):
            # A deadlock can occur when client awaits a response after server's disconnect.
            # ArkTS debugger implementation notifies about execution end via `runtime.ExecutionContextsCleared` event,
            # which is used here to force client disconnect.
            nursery.cancel_scope.cancel()

        return _on_execution_contexts_cleared

    def _get_all_connections(self) -> Iterator[ArkConnection]:
        for conn in list(self._sessions.values()) + [self.connection]:
            yield conn

    def _listen(
        self,
        nursery: trio.Nursery,
        event_handlers: list[Callable[[T], None]],
    ) -> None:
        async def _t() -> None:
            handlers_dict: dict[Any, Callable[[T], None]] = {}
            for handler in event_handlers:
                # Passing `T` as event type will not work,
                # hence type of events are taken from annotations
                args_annotations = getfullargspec(handler).annotations
                event_type = list(args_annotations.values())[0]
                handlers_dict[event_type] = handler
            async for event in self.connection.listen(*tuple(handlers_dict.keys())):
                event_handler = handlers_dict.get(type(event))
                if event_handler is not None:
                    event_handler(event)
                await trio.lowlevel.checkpoint()

        nursery.start_soon(_t)

    async def _setup_receivers(self, event_type: Type[T]):
        receivers = []
        async with self._sessions_lock:
            for session_id, session in self._sessions.items():
                receiver = session.listen(event_type)
                receivers.append((session_id, receiver))
        return receivers

    async def _collect_first(self, session_id, receiver, events_received, nursery):
        async for event in receiver:
            events_received.append((session_id, event))
            nursery.cancel_scope.cancel()

    async def _collect_first_events(self, receivers, events_received):
        async with trio.open_nursery() as nursery:
            for session_id, receiver in receivers:
                nursery.start_soon(self._collect_first, session_id, receiver, events_received, nursery)

    async def _collect_with_deadline(self, session_id, receiver, events_received, deadline):
        async for event in receiver:
            if trio.current_time() > deadline:
                break
            if isinstance(event, debugger.Paused) and event.reason == "Break on start":
                continue
            events_received.append((session_id, event))

    async def _collect_events_with_timeout(self, receivers, events_received, wait_time):
        deadline = trio.current_time() + wait_time
        async with trio.open_nursery() as nursery:
            for session_id, receiver in receivers:
                nursery.start_soon(self._collect_with_deadline, session_id, receiver, events_received, deadline)

            await trio.sleep_until(deadline)
            nursery.cancel_scope.cancel()


class NoPauseDebuggerClient(DebuggerClient):
    def _create_on_debugger_paused(self, connection: ArkConnection):
        async def _on_debugger_paused(event: debugger.Paused) -> None:
            if event.reason == "Break on start":
                await connection.send_and_wait_for(
                    debugger.resume(),
                    debugger.Resumed,
                )
            else:
                await trio.lowlevel.checkpoint()

        return _on_debugger_paused

    def _configure_session_events(self, connection: ArkConnection) -> None:
        super()._configure_session_events(connection)

        async def _t() -> None:
            event_handler = self._create_on_debugger_paused(connection)
            async for event in connection.listen((debugger.Paused)):
                try:
                    await event_handler(event)
                except trio.EndOfChannel:
                    return
                await trio.lowlevel.checkpoint()

        connection.nursery.start_soon(_t)


@asynccontextmanager
async def _create_debugger_client(
    client_type: T,
    connection: ArkConnection,
    scripts: ScriptsCache,
    sources: SourcesCache,
    code_compiler: StringCodeCompiler,
    debugger_config: DebuggerConfig = DebuggerConfig(),
) -> AsyncIterator[DebuggerClient]:
    context = await connection.send_and_wait_for(
        runtime.enable(),
        runtime.ExecutionContextCreated,
    )
    debugger_id = await connection.send(
        debugger.enable(),
    )
    yield client_type(
        connection=connection,
        config=debugger_config,
        debugger_id=debugger_id,
        scripts=scripts,
        sources=sources,
        context=context.context,
        code_compiler=code_compiler,
    )


class BreakpointManager:

    def __init__(self, client: DebuggerClient) -> None:
        self._lock = trio.Lock()
        self.client = client
        self._breaks: Dict[debugger.BreakpointId, List[debugger.Location]] = dict()

    async def set_by_url(self, line_number: int, url: Optional[str]) -> None:
        await self.client.set_breakpoints_active(True)
        br, locs = await self.client.set_breakpoint_by_url(line_number=line_number, url=url)
        async with self._lock:
            self._breaks[br] = locs

    async def get(self, bp_id: debugger.BreakpointId) -> Optional[List[debugger.Location]]:
        async with self._lock:
            return self._breaks.get(bp_id)

    @asynccontextmanager
    async def get_all(self):
        async with self._lock:
            breaks = self._breaks.copy()
        for br, locs in breaks:
            yield (br, locs)
            await trio.lowlevel.checkpoint()

    async def get_possible_breakpoints(self) -> Dict[debugger.BreakpointId, List[debugger.BreakLocation]]:
        # Сhrome does this after set_breakpoint_by_url
        async with self.get_all() as pair:
            return {
                br: br_locs
                async for br, locs in pair
                for br_locs in [
                    await self.client.get_possible_breakpoints(
                        start=dataclasses.replace(loc, column_number=0),
                        end=dataclasses.replace(loc, column_number=1),
                    )
                    for loc in locs
                ]
            }


class DebugLocator:
    scripts: ScriptsCache
    sources: SourcesCache

    def __init__(self, code_compiler: StringCodeCompiler, url: Any) -> None:
        self.code_compiler = code_compiler
        self.url = url
        self.scripts = ScriptsCache()
        self.sources = SourcesCache()

    @asynccontextmanager
    async def connect(self, nursery: trio.Nursery) -> AsyncIterator[DebuggerClient]:
        async with self._connect(DebuggerClient, nursery) as debugger_client:
            yield debugger_client

    @asynccontextmanager
    async def connect_as_no_pauses(self, nursery: trio.Nursery) -> AsyncIterator[NoPauseDebuggerClient]:
        async with self._connect(NoPauseDebuggerClient, nursery) as debugger_client:
            yield debugger_client

    @asynccontextmanager
    async def _connect(self, client_type: T, nursery: trio.Nursery) -> AsyncIterator[T]:
        cdp = await connect_cdp(nursery, self.url, 10)
        async with cdp:
            connection = ArkConnection(cdp, nursery)
            async with _create_debugger_client(
                client_type,
                connection,
                self.scripts,
                self.sources,
                self.code_compiler,
            ) as debugger_client:
                yield debugger_client

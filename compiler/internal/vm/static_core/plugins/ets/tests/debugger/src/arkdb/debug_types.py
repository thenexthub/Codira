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

import base64
from collections import OrderedDict, UserList
from dataclasses import dataclass, field
from pathlib import Path
from subprocess import CalledProcessError
from typing import Any, Dict, Generic, List, Literal, TypeVar, Union

import trio
from cdp import debugger, runtime

from arkdb.compiler import CompileError, EvaluateCompileExpressionArgs, StringCodeCompiler
from arkdb.compiler_verification.expression_verifier import ExpressionVerifier
from arkdb.debug_client import DebuggerClient
from arkdb.logs import logger
from arkdb.mirrors import mirror_array, mirror_object, mirror_primitive, mirror_undefined
from arkdb.source_meta import Breakpoint, SourceMeta

LOG = logger(__name__)

DEFAULT_DEPTH = 3


class Locator:
    def __init__(self, client: DebuggerClient, meta: SourceMeta | None = None) -> None:
        self.client = client
        self.cache: Dict[runtime.RemoteObjectId, PropertyDescriptorCacheEntry] = {}
        self.meta = meta

    async def _get_properties(self, id: runtime.RemoteObjectId) -> List[runtime.PropertyDescriptor]:
        result, _, _, _ = await self.client.get_properties(id, generate_preview=False)
        return result

    async def properties(self, id: runtime.RemoteObjectId) -> "RemoteObjectProperties":
        if (entry := self.cache.get(id, None)) is not None:
            await trio.lowlevel.checkpoint()
            entry.seen.append("other")
            return entry.properties

        props = await self._get_properties(id)
        new_entry = PropertyDescriptorCacheEntry(
            id=id,
            seen=["first"],
            properties=RemoteObjectProperties([PropertyDescriptor(locator=self, data=p) for p in props]),
        )
        self.cache[id] = new_entry
        return new_entry.properties

    def remote_object(self, data: runtime.RemoteObject) -> "RemoteObject":
        return RemoteObject(locator=self, data=data)


T = TypeVar("T")


class Wrap(Generic[T]):
    def __init__(self, locator: Locator, data: T) -> None:
        self.locator = locator
        self.data = data

    @property
    def client(self):
        return self.locator.client

    def __rich_repr__(self):
        yield self.data


class RemoteObjectProperties(UserList["PropertyDescriptor"]):
    def __init__(self, initlist=None):
        super().__init__(initlist)

    async def mirror_values(self, depth: int):
        await trio.lowlevel.checkpoint()
        if depth:
            return {p.name: await p.mirror_value(depth=depth - 1) for p in self.data}
        return EllipsisTerminator()


@dataclass
class PropertyDescriptorCacheEntry:
    id: runtime.RemoteObjectId
    properties: RemoteObjectProperties
    seen: List[str] = field(default_factory=list)


class PropertyDescriptor(Wrap[runtime.PropertyDescriptor]):

    def __init__(self, locator: Locator, data: runtime.PropertyDescriptor) -> None:
        super().__init__(locator, data)

    @property
    def name(self) -> str:
        return self.data.name

    def remote_object(self):
        return self.locator.remote_object(self.data.value) if self.data.value is not None else None

    async def mirror_value(self, *, depth: int) -> Any:
        obj = self.remote_object()
        if obj:
            return await obj.mirror_value(depth=depth)
        await trio.lowlevel.checkpoint()
        return None


class EllipsisTerminator:
    def __repr__(self):
        return "<...>"


class RemoteObject(Wrap[runtime.RemoteObject]):
    async def properties(self) -> RemoteObjectProperties:
        if self.data.object_id is None:
            await trio.lowlevel.checkpoint()
            return RemoteObjectProperties()
        return await self.locator.properties(self.data.object_id)

    async def mirror_value(self, *, depth: int = DEFAULT_DEPTH) -> Any:
        obj = self.data
        match obj.type_:
            case "object":
                return await self._mirror_object(obj, depth=depth)
            case "undefined":
                await trio.lowlevel.checkpoint()
                return mirror_undefined()
            case _:
                await trio.lowlevel.checkpoint()
                return mirror_primitive(obj.type_, obj.value)

    async def _mirror_object(self, obj: runtime.RemoteObject, *, depth: int = DEFAULT_DEPTH) -> Any:
        match obj.subtype:
            case "null":
                await trio.lowlevel.checkpoint()
                return None
            case None:
                if obj.class_name is None:
                    raise RuntimeError(f"class_name is None {obj!r}")
                props = await (await self.properties()).mirror_values(depth=depth)
                if isinstance(props, EllipsisTerminator):
                    return props
                return mirror_object(obj.class_name, **props)
            case "array":
                if obj.class_name is None:
                    raise RuntimeError(f"class_name is None {obj!r}")
                props = await (await self.properties()).mirror_values(depth=depth)
                if isinstance(props, EllipsisTerminator):
                    return [props]
                return mirror_array(obj.class_name, props.values())
            case subtype:
                raise RuntimeError(f"Unexpected subtype '{subtype}'")


class Scope(Wrap[debugger.Scope]):
    async def mirror_variables(self, *, depth: int = DEFAULT_DEPTH) -> OrderedDict[str, Any]:
        return OrderedDict([(p.name, await p.mirror_value(depth=depth)) for p in await self.object_().properties()])

    def object_(self) -> RemoteObject:
        return self.locator.remote_object(self.data.object_)


class Frame(Wrap[debugger.CallFrame]):

    def scope(self, type: Literal["local", "global"] = "local") -> Scope:
        return Scope(
            locator=self.locator,
            data=next(filter(lambda s: s.type_ == type, self.data.scope_chain)),
        )

    def scopes(self):
        for s in self.data.scope_chain:
            yield Scope(locator=self.locator, data=s)

    def this(self):
        return self.locator.remote_object(self.data.this) if self.data.this is not None else None


def compile_expression(
    code_compiler: StringCodeCompiler,
    eval_args: EvaluateCompileExpressionArgs,
    verifier: ExpressionVerifier | None = None,
    allow_compiler_failure: bool = False,
):
    try:
        compiled_expression = code_compiler.compile_expression(eval_args)
    except CalledProcessError as e:
        if allow_compiler_failure:
            raise CompileError(e.output) from e
    if verifier is not None:
        verifier(compiled_expression)
    return base64.b64encode(compiled_expression.panda_file.read_bytes()).decode("utf-8")


class Paused(Wrap[debugger.Paused]):
    def frame(self, id: Union[int, debugger.CallFrameId] = 0) -> Frame:
        frame_id = id if isinstance(id, debugger.CallFrameId) else debugger.CallFrameId(str(id))
        return Frame(
            locator=self.locator,
            data=next(filter(lambda f: f.call_frame_id == frame_id, self.data.call_frames)),
        )

    def frames(self):
        for f in self.data.call_frames:
            yield Frame(locator=self.locator, data=f)

    def local_scope(self) -> Scope:
        return self.frame().scope()

    def hit_breakpoints(self) -> List[Breakpoint]:
        if self.data.hit_breakpoints is None:
            return []
        if self.locator.meta is None:
            return []
        return [
            b for hit in self.data.hit_breakpoints for b in self.locator.meta.get_breakpoint(debugger.BreakpointId(hit))
        ]

    def hit_breakpoint_labels(self):
        return set([b.label for b in self.hit_breakpoints() if b.label])

    async def resume_and_wait_for_paused(self):
        return await self.client.resume_and_wait_for_paused()

    async def evaluate(
        self,
        expression: str | Path,
        abc_files: list[Path],
        verifier: ExpressionVerifier | None = None,
        allow_compiler_failure: bool = False,
    ):
        assert len(self.data.call_frames) > 0
        paused_file = Path(self.data.call_frames[0].url)
        # Must be 1-based.
        paused_code_line = self.data.call_frames[0].location.line_number + 1

        compiled_expression_bytecode = compile_expression(
            self.client.code_compiler,
            EvaluateCompileExpressionArgs(
                ets_expression=expression,
                eval_panda_files=abc_files,
                eval_source=paused_file,
                eval_line=paused_code_line,
                eval_log_level="debug",
                ast_parser=verifier.ast_parser if verifier else None,
            ),
            verifier=verifier,
            allow_compiler_failure=allow_compiler_failure,
        )

        return await self.client.evaluate(compiled_expression_bytecode)


def paused_locator(paused: debugger.Paused, client: DebuggerClient, meta: SourceMeta | None = None) -> Paused:
    return Paused(locator=Locator(client=client, meta=meta), data=paused)

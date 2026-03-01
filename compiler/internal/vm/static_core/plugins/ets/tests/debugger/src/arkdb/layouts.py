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

from collections.abc import Sequence
from pathlib import Path
from typing import Iterable, List, Set

import trio
from rich import box
from rich.columns import Columns
from rich.console import Group, RenderableType
from rich.panel import Panel
from rich.pretty import Pretty
from rich.syntax import Syntax
from rich.table import Column, Table
from rich.text import Text

from .debug_types import DEFAULT_DEPTH, Frame, Paused, Scope


def source_file(src_file: Path, **kwargs) -> Syntax:
    return Syntax.from_path(
        path=str(src_file),
        lexer="ts",
        start_line=0,
        line_numbers=True,
        **kwargs,
    )


async def _add_rows(table: Table, scope: Scope):
    table.add_row(
        f":red_triangle_pointed_down: [table.header]{scope.data.type_}[/table.header]",
        Text(f"{scope.data.object_.description}", style="repr.str"),
    )
    props = await scope.mirror_variables()
    if len(props):
        for k, v in props.items():
            table.add_row(k, Pretty(v))
    else:
        table.add_row("none", "", style="italic")


async def frame_layout(
    frame: Frame,
    scopes: Set[str] | None = None,
    skip_scopes: Sequence[str] | None = None,
) -> RenderableType:
    table = Table(
        Column("name"),
        Column("value"),
        box=box.SIMPLE,
        show_edge=False,
    )
    this = frame.this()
    if this is not None:
        table.add_row("this", Pretty(await this.mirror_value(depth=DEFAULT_DEPTH)))
    else:
        await trio.lowlevel.checkpoint()
    # NOTE(dslynko, #22497): now global scope is ignored by default, because it enumerates all classes from boot.
    # Need to enable enumerating globals after debugger server enumerates only non-boot classes.
    skip_scopes = skip_scopes if skip_scopes is not None else ("global",)
    for scope in frame.scopes():
        if (scopes is None or scope.data.type_ in scopes) and (scope.data.type_ not in skip_scopes):
            await _add_rows(table, scope)
        else:
            table.add_row(
                f":red_triangle_pointed_up: [table.header]{scope.data.type_}[/table.header]",
                f"[repr.str]{scope.data.object_.description}[/repr.str]",
            )

    title = "[inspect.class]%s[/inspect.class] [repr.filename]%s[/repr.filename]:[repr.number]%s[/repr.number]" % (
        frame.data.function_name,
        Path(frame.data.url).name,
        frame.data.location.line_number,
    )
    return Panel(table, title=title, title_align="left")


async def frames_layout(
    call_frames: Iterable[Frame],
    scopes: Set[str] | None = None,
    skip_scopes: Sequence[str] | None = None,
) -> List[RenderableType]:

    await trio.lowlevel.checkpoint()
    return [await frame_layout(f, scopes, skip_scopes) for f in call_frames]


async def paused_layout(
    paused: Paused,
    url: Path,
    scopes: Set[str] | None = None,
    skip_scopes: Sequence[str] | None = None,
) -> RenderableType:
    frames = await frames_layout(paused.frames(), scopes=scopes, skip_scopes=skip_scopes)
    highlight_lines = [f.location.line_number for f in paused.data.call_frames]
    code = source_file(url, highlight_lines=highlight_lines)
    return Columns([code, Group(*frames)])

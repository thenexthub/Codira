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

import re
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

from cdp import debugger, runtime


def create_pattern():
    """
    Examples:
        function main(): int {
            let a: int = 100;    //#BP{}
            let b: int = a + 10; //  #BP{label}
            console.log(a + 1)   //  # BP
            return a;            // #BP {}
        }
    """
    return re.compile(r"//\s+#\s*(?P<br>BP\s*(\{(?P<br_label>.*?)\})?)\s*$")


BREAKPOINT_PATTERN = create_pattern()


@dataclass
class Breakpoint:
    line_number: int
    label: Optional[str]
    breakpoint_id: debugger.BreakpointId | None = None
    locations: List[debugger.Location] | None = None


@dataclass
class SourceMeta:
    breakpoints: List[Breakpoint]

    def locations(self, script_id: runtime.ScriptId):
        return breakpoints_to_locations(self.breakpoints, script_id)

    def get_breakpoint(self, breakpoint_id: debugger.BreakpointId) -> List[Breakpoint]:
        return [bp for bp in self.breakpoints if bp.breakpoint_id == breakpoint_id]


def parse_source_meta(source: str | List[str] | Path) -> SourceMeta:
    lines: List[str] = []
    if isinstance(source, str):
        lines = source.splitlines()
    elif isinstance(source, Path):
        with source.open(mode="r") as f:
            lines = f.readlines()
    else:
        lines = source
    brs = []
    for line_number, line in enumerate(lines):
        match = BREAKPOINT_PATTERN.search(line)
        if match:
            groups = match.groupdict()
            if "br" in groups:
                brs.append(Breakpoint(line_number=line_number, label=groups.get("br_label")))
    return SourceMeta(breakpoints=brs)


def breakpoints_to_locations(breakpoints: List[Breakpoint], script_id: runtime.ScriptId):
    return [
        (
            debugger.Location(script_id=script_id, line_number=br.line_number),
            br.label,
        )
        for br in breakpoints
    ]

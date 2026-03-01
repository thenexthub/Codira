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

from __future__ import annotations
from dataclasses import dataclass
import typing

import cdp.debugger as debugger
import cdp.runtime as runtime
import cdp.util as util


@dataclass
class CustomUrlBreakpointResponse:
    id_: debugger.BreakpointId

    #: Line number in the script (0-based).
    line_number: int

    #: Column number in the script (0-based).
    column_number: int

    #: Script identifier as reported in the ``Debugger.scriptParsed``.
    script_id: runtime.ScriptId

    @classmethod
    def from_json(cls, json: util.T_JSON_DICT) -> CustomUrlBreakpointResponse:
        return cls(
            id_=debugger.BreakpointId.from_json(json["id"]),
            line_number=int(json["lineNumber"]),
            column_number=int(json["columnNumber"]),
            script_id=runtime.ScriptId.from_json(json["scriptId"]),
        )

    def to_json(self) -> util.T_JSON_DICT:
        json: util.T_JSON_DICT = dict()
        json["id"] = self.id_
        json["lineNumber"] = self.line_number
        json["columnNumber"] = self.column_number
        json["scriptId"] = self.script_id.to_json()
        return json


@dataclass
class UrlBreakpointRequest:
    line_number: int
    url: typing.Optional[str] = None
    url_regex: typing.Optional[str] = None
    column_number: typing.Optional[int] = None
    script_hash: typing.Optional[str] = None
    condition: typing.Optional[str] = None

    @classmethod
    def from_json(cls, json: util.T_JSON_DICT) -> UrlBreakpointRequest:
        return cls(
            line_number=int(json["lineNumber"]),
            url=str(json["url"]) if "url" in json else None,
            url_regex=str(json["urlRegex"]) if "urlRegex" in json else None,
            column_number=int(json["columnNumber"]) if "columnNumber" in json else None,
            script_hash=str(json["scriptHash"]) if "scriptHash" in json else None,
            condition=str(json["condition"]) if "condition" in json else None,
        )

    def to_json(self) -> util.T_JSON_DICT:
        json: util.T_JSON_DICT = dict()
        json["lineNumber"] = self.line_number
        if self.url is not None:
            json["url"] = self.url
        if self.url_regex is not None:
            json["urlRegex"] = self.url_regex
        if self.column_number is not None:
            json["columnNumber"] = self.column_number
        if self.script_hash is not None:
            json["scriptHash"] = self.script_hash
        if self.condition is not None:
            json["condition"] = self.condition
        return json


def remove_breakpoints_by_url(url: str) -> typing.Generator[util.T_JSON_DICT, util.T_JSON_DICT, None]:
    """
    Removes all ArkTS breakpoints from the file.

    :param url:
    """
    params: util.T_JSON_DICT = dict()
    params["url"] = url
    cmd_dict: util.T_JSON_DICT = {
        "method": "Debugger.removeBreakpointsByUrl",
        "params": params,
    }
    yield cmd_dict


def get_possible_and_set_breakpoint_by_url(
    locations: typing.List[UrlBreakpointRequest],
) -> typing.Generator[util.T_JSON_DICT, util.T_JSON_DICT, typing.List[CustomUrlBreakpointResponse]]:
    """
    Set breakpoint in a batch manner

    :param locations: to set breakpoints at
    """
    params: util.T_JSON_DICT = dict()
    params["locations"] = [loc.to_json() for loc in locations]
    cmd_dict: util.T_JSON_DICT = {
        "method": "Debugger.getPossibleAndSetBreakpointByUrl",
        "params": params,
    }
    json = yield cmd_dict
    return [CustomUrlBreakpointResponse.from_json(i) for i in json["locations"]]

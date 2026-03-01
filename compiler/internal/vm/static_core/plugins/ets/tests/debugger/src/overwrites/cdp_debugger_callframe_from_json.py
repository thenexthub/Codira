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

import typing

import cdp.debugger as debugger
import cdp.runtime as runtime

# CallFrame not containts `this`


def overwrite() -> None:
    """
    The debugger does not return the required field `this`.
    """

    def fix_callframe_from_json(json: typing.Dict[str, typing.Any]) -> debugger.CallFrame:
        return debugger.CallFrame(
            call_frame_id=debugger.CallFrameId.from_json(json["callFrameId"]),
            function_name=str(json["functionName"]),
            location=debugger.Location.from_json(json["location"]),
            url=str(json["url"]),
            scope_chain=[debugger.Scope.from_json(i) for i in json["scopeChain"]],
            this=runtime.RemoteObject.from_json(json["this"]) if "this" in json else None,  # type: ignore
            function_location=(
                debugger.Location.from_json(json["functionLocation"]) if "functionLocation" in json else None
            ),
            return_value=runtime.RemoteObject.from_json(json["returnValue"]) if "returnValue" in json else None,
        )

    setattr(debugger.CallFrame, "from_json", fix_callframe_from_json)

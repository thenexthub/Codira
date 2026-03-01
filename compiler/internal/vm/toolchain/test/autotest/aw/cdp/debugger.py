#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Python CDP Debugger.
"""

from dataclasses import dataclass, field
from enum import Enum
from typing import Optional, List


@dataclass
class EnableAccelerateLaunchParams:
    options = ['enableLaunchAccelerate']
    max_scripts_cache_size: float = 1e7


@dataclass
class SaveAllPossibleBreakpointsParams:
    locations: dict


@dataclass
class DropFrameParams:
    dropped_depth: int = 1


@dataclass
class ReplyNativeCallingParams:
    user_code: bool = True


@dataclass
class SetMixedDebugEnabledParams:
    enabled: bool
    mixed_stack_enabled: bool


@dataclass
class SmartStepIntoParams:
    url: str
    line_number: int


@dataclass
class PauseOnExceptionsState(Enum):
    ALL = 'all'
    NONE = 'none'
    CAUGHT = 'caught'
    UNCAUGHT = 'uncaught'


@dataclass
class EvaluateOnCallFrameParams:
    expression: str
    call_frame_id: int = 0
    object_group: str = "console"
    include_command_line_api: bool = True
    silent: bool = True


@dataclass
class BreakLocationUrl:
    url: str
    line_number: int
    column_number: Optional[int] = 0
    condition: Optional[str] = None

    def to_json(self):
        json = {'url': self.url,
                'lineNumber': self.line_number,
                'columnNumber': self.column_number}
        if self.condition is not None:
            json['condition'] = self.condition
        return json

@dataclass
class SymbolicBreakpoint:
    functionName: str

    def to_json(self):
        return {'functionName': self.functionName}

@dataclass
class RemoveBreakpointsUrl:
    url: str = ""


@dataclass
class SetBreakpointsLocations:
    locations: list = field(default_factory=list)

@dataclass
class SymbolicBreakpoints:
    symbolicBreakpoints: List[SymbolicBreakpoint] = field(default_factory=list)

def enable(params: EnableAccelerateLaunchParams | None):
    command = {'method': 'Debugger.enable'}
    if params is not None:
        command['params'] = {
            'options': params.options,
            'maxScriptsCacheSize': params.max_scripts_cache_size
        }
    return command

def enable_static(params: EnableAccelerateLaunchParams | None):
    # params is reserved for future extension
    _ = params
    command = {'method': 'Debugger.enable',
               'params': {},
               "sessionId": ''}
    return command

def resume():
    command = {'method': 'Debugger.resume'}
    return command

def resume_static():
    command = {'method': 'Debugger.resume',
               "sessionId": ''}
    return command

def remove_breakpoints_by_url(params: RemoveBreakpointsUrl):
    command = {'method': 'Debugger.removeBreakpointsByUrl',
               'params': {'url': params.url}}
    return command

def remove_breakpoints_by_url_static(params: RemoveBreakpointsUrl):
    command = {'method': 'Debugger.removeBreakpointsByUrl',
               'params': {'url': params.url},
               "sessionId": ''}
    return command

def get_possible_and_set_breakpoint_by_url(params: SetBreakpointsLocations):
    locations = []
    for location in params.locations:
        locations.append(location.to_json())
    command = {'method': 'Debugger.getPossibleAndSetBreakpointByUrl',
               'params': {'locations': locations}}
    return command

def get_possible_and_set_breakpoint_by_url_static(params: SetBreakpointsLocations):
    locations = []
    for location in params.locations:
        locations.append(location.to_json())
    command = {'method': 'Debugger.getPossibleAndSetBreakpointByUrl',
               'params': {'locations': locations},
               "sessionId": ''}
    return command

def set_symbolic_breakpoints(params: SymbolicBreakpoints):
    symbolicBreakpoints = []
    for symbolicBreakpoint in params.symbolicBreakpoints:
        symbolicBreakpoints.append(symbolicBreakpoint.to_json())
    command = {'method': 'Debugger.setSymbolicBreakpoints',
               'params': {'symbolicBreakpoints': symbolicBreakpoints}}
    return command

def remove_symbolic_breakpoints(params: SymbolicBreakpoints):
    symbolicBreakpoints = []
    for symbolicBreakpoint in params.symbolicBreakpoints:
        symbolicBreakpoints.append(symbolicBreakpoint.to_json())
    command = {'method': 'Debugger.removeSymbolicBreakpoints',
               'params': {'symbolicBreakpoints': symbolicBreakpoints}}
    return command

def step_over():
    command = {'method': 'Debugger.stepOver'}
    return command

def step_over_static():
    command = {'method': 'Debugger.stepOver',
               "sessionId": ''}
    return command

def step_into():
    command = {'method': 'Debugger.stepInto'}
    return command

def step_into_static(sessionId = ""):
    command = {'method': 'Debugger.stepInto',
               "sessionId": sessionId}
    return command

def step_out():
    command = {'method': 'Debugger.stepOut'}
    return command

def step_out_static():
    command = {'method': 'Debugger.stepOut',
               "sessionId": ''}
    return command

def disable():
    command = {'method': 'Debugger.disable'}
    return command

def disable_static():
    command = {'method': 'Debugger.disable',
               "sessionId": ''}
    return command

def set_pause_on_exceptions(params: PauseOnExceptionsState):
    command = {'method': 'Debugger.setPauseOnExceptions',
               'params': {'state': params.value}}
    return command


def evaluate_on_call_frame(params: EvaluateOnCallFrameParams):
    command = {'method': 'Debugger.evaluateOnCallFrame',
               'params': {
                   'callFrameId': str(params.call_frame_id),
                   'expression': params.expression,
                   'includeCommandLineApi': params.include_command_line_api,
                   'objectGroup': params.object_group,
                   'silent': params.silent}}
    return command


def pause():
    command = {'method': 'Debugger.pause'}
    return command


def set_mixed_debug_enabled(params: SetMixedDebugEnabledParams):
    command = {'method': 'Debugger.setMixedDebugEnabled',
               'params': {'enabled': params.enabled, 'mixedStackEnabled': params.mixed_stack_enabled}}
    return command


def reply_native_calling(params: ReplyNativeCallingParams):
    command = {'method': 'Debugger.replyNativeCalling',
               'params': {'userCode': params.user_code}}
    return command


def drop_frame(params: DropFrameParams):
    command = {'method': 'Debugger.dropFrame',
               'params': {'droppedDepth': params.dropped_depth}}
    return command


def smart_step_into(params: SmartStepIntoParams):
    command = {'method': 'Debugger.smartStepInto',
               'params': {'url': params.url, 'lineNumber': params.line_number}}
    return command


def save_all_possible_breakpoints(params: SaveAllPossibleBreakpointsParams):
    locations = {}
    for key, value in params.locations.items():
        positions = []
        for pos in value:
            if isinstance(pos, int):
                positions.append({"lineNumber": pos, "columnNumber": 0})
            else:
                positions.append({"lineNumber": pos[0], "columnNumber": pos[1]})
        locations[key] = positions
    command = {'method': 'Debugger.saveAllPossibleBreakpoints',
               'params': {'locations': locations}}
    return command

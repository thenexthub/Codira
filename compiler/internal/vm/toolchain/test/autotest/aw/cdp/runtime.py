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

Description: Python CDP Runtime.
"""

from dataclasses import dataclass
from typing import Optional


@dataclass
class GetPropertiesParams:
    object_id: str = ""
    own_properties: bool = True
    accessor_properties_only: bool = False
    generate_preview: bool = True
    start_index: Optional[int] = None
    group_count: Optional[int] = None
    non_indexed_properties_only: Optional[bool] = None


@dataclass
class PropertiesParamsStatic():
    object_id: str = ""
    own_properties: bool = True
    accessor_properties_only: bool = False
    generate_preview: bool = True
    non_indexed_properties_only: Optional[bool] = None
    sessionId: str = ""

def enable():
    command = {'method': 'Runtime.enable'}
    return command

def enable_static():
    command = {'method': 'Runtime.enable',
               'params': {},
               "sessionId": ''}
    return command

def run_if_waiting_for_debugger():
    command = {'method': 'Runtime.runIfWaitingForDebugger'}
    return command


def get_properties(params: GetPropertiesParams):
    command = {'method': 'Runtime.getProperties',
               'params': {'objectId': params.object_id,
                          'ownProperties': params.own_properties,
                          'accessorPropertiesOnly': params.accessor_properties_only,
                          'generatePreview': params.generate_preview}}
    if params.non_indexed_properties_only is not None:
        command['params']['nonIndexedPropertiesOnly'] = params.non_indexed_properties_only
    if params.start_index is not None:
        command['params']['start'] = params.start_index
    if params.group_count is not None:
        command['params']['count'] = params.group_count
    return command

def get_properties_static(params: PropertiesParamsStatic, sessionId: ""):
    command = {'method': 'Runtime.getProperties',
               'params': {'objectId': params.object_id,
                          'ownProperties': params.own_properties,
                          'accessorPropertiesOnly': params.accessor_properties_only,
                          'generatePreview': params.generate_preview},
               "sessionId": sessionId}
    if params.non_indexed_properties_only is not None:
        command['params']['nonIndexedPropertiesOnly'] = params.non_indexed_properties_only
    return command


def get_heap_usage():
    return {'method': 'Runtime.getHeapUsage'}

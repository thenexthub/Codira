#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Python Profiler Domain Interfaces
"""

import re
import sys
import json
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent)) # add aw path to sys.path

from all_utils import CommonUtils
from implement_api.protocol_api import ProtocolImpl
from cdp import profiler
from customized_types import ProtocolType

comm_with_debugger_server = CommonUtils.communicate_with_debugger_server


class ProfilerImpl(ProtocolImpl):

    def __init__(self, id_generator, websocket):
        super().__init__(id_generator, websocket)
        self.dispatch_table = {"start": (self.start, ProtocolType.send),
                               "stop": (self.stop, ProtocolType.send),
                               "setSamplingInterval": (self.set_sampling_interval, ProtocolType.send)}

    async def start(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   profiler.start(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def stop(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   profiler.stop(), message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        time_deltas = response['result']['profile']['timeDeltas']
        assert all(i >= 0 for i in time_deltas), \
            f"TimeDeltas are not correct: {time_deltas}"
        nodes = response['result']['profile']['nodes']
        # NAPI方法名需要遵循格式: 方法名(地址)(NAPI)
        pattern = r'^[\w.]+\([A-Za-z0-9]+\)\(NAPI\)$'
        for node in nodes:
            func_name = node['callFrame']['functionName']
            if func_name.endswith('(NAPI)'):
                assert re.match(pattern, func_name), \
                    f"The function name '{func_name}' is not match the pattern '{pattern}'"
        return response

    async def set_sampling_interval(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   profiler.set_sampling_interval(params), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

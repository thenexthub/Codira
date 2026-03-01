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

Description: Python Runtime Domain Interfaces
"""

import sys
import json
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent)) # add aw path to sys.path

from all_utils import CommonUtils
from implement_api.protocol_api import ProtocolImpl
from cdp import runtime
from customized_types import ProtocolType

comm_with_debugger_server = CommonUtils.communicate_with_debugger_server


class RuntimeImpl(ProtocolImpl):

    def __init__(self, id_generator, websocket):
        super().__init__(id_generator, websocket)
        self.dispatch_table = {"enable": (self.enable, ProtocolType.send),
                               "enableStatic": (self.enable_static, ProtocolType.send),
                               "runIfWaitingForDebugger": (self.run_if_waiting_for_debugger, ProtocolType.send),
                               "getProperties": (self.get_properties, ProtocolType.send),
                               "getPropertiesStatic": (self.get_properties_static, ProtocolType.send),
                               "getHeapUsage": (self.get_heap_usage, ProtocolType.send)}

    async def enable(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   runtime.enable(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {"protocol": []}})

    async def enable_static(self, message_id, connection, params, is_hybrid=False, counts=1, sessionId=""):
        response = await comm_with_debugger_server(self.websocket,
                                                   connection,
                                                   runtime.enable_static(),
                                                   message_id,
                                                   counts)

    async def run_if_waiting_for_debugger(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   runtime.run_if_waiting_for_debugger(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def get_properties(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   runtime.get_properties(params), message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

    async def get_properties_static(self, message_id, connection, params, is_hybrid=False, counts=1, sessionId=""):
        response = await comm_with_debugger_server(self.websocket,
                                                   connection,
                                                   runtime.get_properties_static(params, sessionId),
                                                   message_id)
        response = json.loads(response)
        return response

    async def get_heap_usage(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   runtime.get_heap_usage(), message_id)
        while response.startswith('{"method":"HeapProfiler.lastSeenObjectId"') \
                or response.startswith('{"method":"HeapProfiler.heapStatsUpdate"'):
            response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                        connection.received_msg_queue)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        assert response['result']['usedSize'] > 0, f"{response['result']['usedSize']} is not greater than 0"
        assert response['result']['totalSize'] > 0, f"{response['result']['totalSize']} is not greater than 0"
        return response

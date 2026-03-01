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

Description: Python Debugger Domain Interfaces
"""

import sys
import json
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent)) # add aw path to sys.path

from all_utils import CommonUtils
from implement_api.protocol_api import ProtocolImpl
from cdp import debugger
from customized_types import ThreadConnectionInfo, ProtocolType

comm_with_debugger_server = CommonUtils.communicate_with_debugger_server


class DebuggerImpl(ProtocolImpl):

    def __init__(self, id_generator, websocket):
        super().__init__(id_generator, websocket)
        self.dispatch_table = {"enable": (self.enable, ProtocolType.send),
                               "enableStatic": (self.enable, ProtocolType.send),
                               "disable": (self.disable, ProtocolType.send),
                                "disableStatic": (self.disable_static, ProtocolType.send),
                               "resume": (self.resume, ProtocolType.send),
                               "pause": (self.pause, ProtocolType.send),
                               "removeBreakpointsByUrl": (self.remove_breakpoints_by_url, ProtocolType.send),
                               "getPossibleAndSetBreakpointsByUrl": (self.get_possible_and_set_breakpoints_by_url,
                                                                     ProtocolType.send),
                               "setSymbolicBreakpoints": (self.set_symbolic_breakpoints, ProtocolType.send),
                               "removeSymbolicBreakpoints": (self.remove_symbolic_breakpoints, ProtocolType.send),
                               "stepOver": (self.step_over, ProtocolType.send),
                               "stepInto": (self.step_into, ProtocolType.send),
                               "stepOut": (self.step_out, ProtocolType.send),
                               "setPauseOnExceptions": (self.set_pause_on_exceptions, ProtocolType.send),
                               "evaluateOnCallFrame": (self.evaluate_on_call_frame, ProtocolType.send),
                               "smartStepInto": (self.smart_step_into, ProtocolType.send),
                               "setMixedDebugEnabled": (self.set_mixed_debug_enabled, ProtocolType.send),
                               "replyNativeCalling": (self.reply_native_calling, ProtocolType.send),
                               "dropFrame": (self.drop_frame, ProtocolType.send),
                               "saveAllPossibleBreakpoints": (self.save_all_possible_breakpoints, ProtocolType.send),
                               "scriptParsed": (self.script_parsed, ProtocolType.recv),
                               "paused": (self.paused, ProtocolType.recv),
                               "nativeCalling": (self.native_calling, ProtocolType.recv)}

    async def connect_to_debugger_server(self, pid, is_main=True):
        if is_main:
            send_msg = {"type": "connected"}
            await self.websocket.send_msg_to_connect_server(send_msg)
            response = await self.websocket.recv_msg_of_connect_server()
            response = json.loads(response)
            CommonUtils.assert_equal(response['type'], 'addInstance')
            CommonUtils.assert_equal(response['instanceId'], 0)
            CommonUtils.assert_equal(response['tid'], pid)
        else:
            response = await self.websocket.recv_msg_of_connect_server()
            response = json.loads(response)
            CommonUtils.assert_equal(response['type'], 'addInstance')
            assert response['instanceId'] != 0, f"Worker instanceId can not be 0"
            assert response['tid'] != pid, f"Worker tid can not be {pid}"
            assert 'workerThread_' in response['name'], f"'workerThread_' not in {response['name']}"
        instance_id = await self.websocket.get_instance()
        send_queue = self.websocket.to_send_msg_queues[instance_id]
        recv_queue = self.websocket.received_msg_queues[instance_id]
        connection = ThreadConnectionInfo(instance_id, send_queue, recv_queue)
        return connection

    async def destroy_instance(self):
        response = await self.websocket.recv_msg_of_connect_server()
        response = json.loads(response)
        CommonUtils.assert_equal(response['type'], 'destroyInstance')
        return response

    async def enable(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket,
                                                   connection,
                                                   debugger.enable(params),
                                                   message_id,
                                                   counts)
        while response.startswith('{"method":"Debugger.scriptParsed"'):
            response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                        connection.received_msg_queue)
        # 暂时删除对response的校验,以适配不同版本

    async def enable_static(self, message_id, connection, params, is_hybrid=False, counts=1, sessionId=""):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.enable_static(params), message_id, counts)
        while response.startswith('{"method":"Debugger.scriptParsed"'):
            response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                        connection.received_msg_queue)
        # 暂时删除对response的校验，以适配不同版本

    async def disable(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.disable(), message_id)
        while response.startswith('{"method":"Debugger.resumed"') \
                or response.startswith('{"method":"HeapProfiler.lastSeenObjectId"') \
                or response.startswith('{"method":"HeapProfiler.heapStatsUpdate"'):
            response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                        connection.received_msg_queue)
        expected_response = {"id": message_id, "result": {}}
        CommonUtils.assert_equal(json.loads(response), expected_response)

    async def disable_static(self, message_id, connection, params, is_hybrid=False, counts=1, sessionId=""):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.disable_static(), message_id)
        while response.startswith('{"method":"Debugger.resumed"') \
                or response.startswith('{"method":"HeapProfiler.lastSeenObjectId"') \
                or response.startswith('{"method":"HeapProfiler.heapStatsUpdate"'):
            response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                        connection.received_msg_queue)
    async def script_parsed(self, connection, params):
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        response = json.loads(response)
        return response

    async def paused(self, connection, params):
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        response = json.loads(response)
        CommonUtils.assert_equal(response['method'], 'Debugger.paused')
        return response

    async def resume(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.resume(), message_id)
        expected_response = {"method": "Debugger.resumed", "params": {}}
        CommonUtils.assert_equal(json.loads(response), expected_response)
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        expected_response = {"id": message_id, "result": {}}
        CommonUtils.assert_equal(json.loads(response), expected_response)

    async def remove_breakpoints_by_url(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.remove_breakpoints_by_url(params), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def get_possible_and_set_breakpoints_by_url(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.get_possible_and_set_breakpoint_by_url(params),
                                                   message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

    async def set_symbolic_breakpoints(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.set_symbolic_breakpoints(params),
                                                   message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

    async def remove_symbolic_breakpoints(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.remove_symbolic_breakpoints(params),
                                                   message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

    async def step_over(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.step_over(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"method": "Debugger.resumed", "params": {}})
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def step_out(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.step_out(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"method": "Debugger.resumed", "params": {}})
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def step_into(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.step_into(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"method": "Debugger.resumed", "params": {}})
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def set_pause_on_exceptions(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.set_pause_on_exceptions(params), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def evaluate_on_call_frame(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.evaluate_on_call_frame(params), message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

    async def pause(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.pause(), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def smart_step_into(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.smart_step_into(params), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def set_mixed_debug_enabled(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.set_mixed_debug_enabled(params), message_id)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def native_calling(self, connection, params):
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        response = json.loads(response)
        CommonUtils.assert_equal(response['method'], 'Debugger.nativeCalling')
        return response

    async def reply_native_calling(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.reply_native_calling(params), message_id)
        CommonUtils.assert_equal(json.loads(response), {"method": "Debugger.resumed", "params": {}})
        response = await self.websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                    connection.received_msg_queue)
        CommonUtils.assert_equal(json.loads(response), {"id": message_id, "result": {}})

    async def drop_frame(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.drop_frame(params), message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

    async def save_all_possible_breakpoints(self, message_id, connection, params):
        response = await comm_with_debugger_server(self.websocket, connection,
                                                   debugger.save_all_possible_breakpoints(params), message_id)
        response = json.loads(response)
        CommonUtils.assert_equal(response['id'], message_id)
        return response

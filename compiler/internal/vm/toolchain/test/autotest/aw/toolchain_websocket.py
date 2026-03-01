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

Description: Responsible for websocket communication.
"""

import asyncio
import json
import time

from install_lib import install

install('websockets')
from fport import Fport
import websockets.protocol
from websockets import connect, ConnectionClosed
from customized_types import ThreadConnectionInfo

class ToolchainWebSocket(object):
    def __init__(self, driver, config, print_protocol=True):
        self.driver = driver
        self.server_ip = driver._device.host
        self.config = config
        self.fport = Fport(self.driver)
        self.increase_step = 7
        self.debugger_server_connection_threshold = 3

        self.to_send_msg_queue_for_connect_server = None
        self.received_msg_queue_for_connect_server = None

        self.to_send_msg_queues = {}   # key: instance_id, value: to_send_msg_queue
        self.received_msg_queues = {}  # key: instance_id, value: received_msg_queue
        self.debugger_server_instance = None
        self.new_instance_flag = True
        self.log = driver.log_info if print_protocol else (lambda s: None)

    async def drain_remaining_messages(self, instance_id, queue, remaining_counts, extra_count, drain_timeout):
        """Drain up to 'remaining_counts' messages from the queue with short timeout."""
        local_count = remaining_counts
        local_extra = extra_count
        while local_count > 0:
            try:
                msg = await asyncio.wait_for(queue.get(), timeout=drain_timeout)
                queue.task_done()
                self.log(f'[<==] Instance {instance_id} counts DRAINED message: {msg}')
                local_count -= 1
                local_extra += 1
            except asyncio.TimeoutError:
                break  # No more messages within timeout
        return local_extra

    async def recv_msg_of_debugger_server(self, instance_id, queue, counts=1, timeout=60, drain_timeout=0.3):
        # counts:  represents the number of recv messages corresponding to one send message.

        # 1. Wait for the first message
        first_msg = await asyncio.wait_for(queue.get(), timeout=timeout)
        queue.task_done()
        # 2. Target.attachedToTarget and Target.detachedFromTarget will drain
        while 'Target.att' in first_msg or 'Target.det' in first_msg:
            self.log(f'[<==] Instance {instance_id} DRAINED message: {first_msg}')
            first_msg = await asyncio.wait_for(queue.get(), timeout=timeout)
            queue.task_done()

        self.log(f'[<==] Instance {instance_id} receive FIRST message: {first_msg}')
        counts -= 1
        # 3. Drain any additional expected messages
        extra_count = 0
        if counts > 0:
            extra_count = await self.drain_remaining_messages(
                instance_id,
                queue,
                counts,
                extra_count,
                drain_timeout
            )

        # 4. return the first message
        return first_msg

    async def send_msg_to_debugger_server(self, instance_id, queue, message):
        await queue.put(message)
        self.log(f'[==>] Instance {instance_id} send message: {message}')
        return True

    async def get_instance(self):
        instance_id = await self.debugger_server_instance.get()
        self.debugger_server_instance.task_done()
        return instance_id

    def no_more_instance(self):
        self.new_instance_flag = False

    async def recv_msg_of_connect_server(self):
        message = await self.received_msg_queue_for_connect_server.get()
        self.received_msg_queue_for_connect_server.task_done()
        return message

    async def send_msg_to_connect_server(self, message):
        await self.to_send_msg_queue_for_connect_server.put(message)
        self.log(f'[==>] Connect server send message: {message}')
        return True

    async def main_task(self, taskpool, procedure, pid):
        # the async queue must be initialized in task
        self.to_send_msg_queue_for_connect_server = asyncio.Queue()
        self.received_msg_queue_for_connect_server = asyncio.Queue()
        self.debugger_server_instance = asyncio.Queue(maxsize=1)

        connect_server_client = await self._connect_connect_server()
        taskpool.submit(self._sender(connect_server_client, self.to_send_msg_queue_for_connect_server))
        taskpool.submit(self._receiver_of_connect_server(connect_server_client,
                                                         self.received_msg_queue_for_connect_server,
                                                         taskpool, pid))
        taskpool.submit(procedure(self))

    async def _sender(self, client, send_queue):
        assert client.state == websockets.protocol.OPEN, f'Client state of _sender is: {client.state}'
        while True:
            send_message = await send_queue.get()
            send_queue.task_done()
            if send_message == 'close':
                await client.close(reason='close')
                return
            await client.send(json.dumps(send_message))

    async def _receiver(self, client, received_queue):
        assert client.state == websockets.protocol.OPEN, f'Client state of _receiver is: {client.state}'
        while True:
            try:
                response = await client.recv()
                await received_queue.put(response)
            except ConnectionClosed as e:
                err_log = 'Debugger server connection closed'
                if e.rcvd is not None:
                    err_log += f': code = {e.rcvd.code}, reason = {e.rcvd.reason}'
                self.log(err_log)
                return

    async def _connect_server(self, port):
        client = None
        try:
            client = await connect(f'ws://{self.server_ip}:{port}')
        except Exception as e:
            self.log(f"Connect to {self.server_ip}:{port} failed. {type(e).__name__}:{e}")
        return client

    async def _connect_connect_server(self, max_retries=3):
        self.fport.clear_fport()
        while max_retries:
            port, fport_info = self.fport.fport_connect_server(self.config['connect_server_port'],
                                                               self.config['pid'],
                                                               self.config['bundle_name'])
            assert port > 0, 'Failed to fport connectServer'
            client = await self._connect_server(port)
            if client is not None:
                return client
            max_retries -= 1
            self.fport.rm_fport_server(fport_info)
            self.config['connect_server_port'] = port + self.increase_step
        assert max_retries > 0, 'Failed to connect connectServer'

    async def _connect_debugger_server(self, instance_id, max_retries=3):
        while max_retries:
            port, fport_info = self.fport.fport_debugger_server(self.config['debugger_server_port'],
                                                                self.config['pid'],
                                                                instance_id)
            assert port > 0, 'Failed to fport debuggerServer'
            client = await self._connect_server(port)
            if client is not None:
                return client
            max_retries -= 1
            self.fport.rm_fport_server(fport_info)
            self.config['debugger_server_port'] = port + self.increase_step
        assert max_retries > 0, 'Failed to connect debuggerServer'

    async def _receiver_of_connect_server(self, client, receive_queue, taskpool, pid):
        assert client.state == websockets.protocol.OPEN, \
            f'Client state of _receiver_of_connect_server is: {client.state}'
        num_debugger_server_client = 0
        while True:
            try:
                response = await client.recv()
                await receive_queue.put(response)
                self.log(f'[<==] Connect server receive message: {response}')
                response = json.loads(response)

                # The debugger server client is only responsible for adding and removing instances
                if (response['type'] == 'addInstance' and self.new_instance_flag and
                        num_debugger_server_client < self.debugger_server_connection_threshold):
                    instance_id = response['instanceId']
                    debugger_server_client = await self._connect_debugger_server(instance_id)
                    self.log(f'InstanceId: {instance_id}, port: {self.config["debugger_server_port"]}, '
                             f'debugger server connected')
                    self.config["debugger_server_port"] += 1

                    to_send_msg_queue = asyncio.Queue()
                    received_msg_queue = asyncio.Queue()
                    self.to_send_msg_queues[instance_id] = to_send_msg_queue
                    self.received_msg_queues[instance_id] = received_msg_queue
                    taskpool.submit(coroutine=self._sender(debugger_server_client, to_send_msg_queue))
                    taskpool.submit(coroutine=self._receiver(debugger_server_client, received_msg_queue))

                    await self._store_instance(instance_id)
                    num_debugger_server_client += 1

                elif response['type'] == 'destroyInstance':
                    instance_id = response['instanceId']
                    to_send_msg_queue = self.to_send_msg_queues[instance_id]
                    await self.send_msg_to_debugger_server(instance_id, to_send_msg_queue, 'close')
                    num_debugger_server_client -= 1

            except ConnectionClosed as e:
                err_log = 'Connect server connection closed'
                if e.rcvd is not None:
                    err_log += f': code = {e.rcvd.code}, reason = {e.rcvd.reason}'
                self.log(err_log)
                return

    async def _store_instance(self, instance_id):
        await self.debugger_server_instance.put(instance_id)
        return True

    async def connect_to_debugger_server(self, pid, is_main=True, is_hybrid=False):
        if is_hybrid:
            send_msg = {"type": "connected"}
            await self.send_msg_to_connect_server(send_msg)
            response = await self.recv_msg_of_connect_server()
            response = json.loads(response)
            assert response['type'] == 'addInstance', f"\nExpected: addInstance\nActual: {response['type']}"
            assert response['instanceId'] == pid, f"\nExpected: {pid}\nActual: {response['instanceId']}"
            assert response['tid'] == pid, f"\nExpected: {pid}\nActual: {response['tid']}"
        elif is_main:
            send_msg = {"type": "connected"}
            await self.send_msg_to_connect_server(send_msg)
            response = await self.recv_msg_of_connect_server()
            response = json.loads(response)
            assert response['type'] == 'addInstance', f"\nExpected: addInstance\nActual: {response['type']}"
            assert response['instanceId'] == 0, f"\nExpected: 0\nActual: {response['instanceId']}"
            assert response['tid'] == pid, f"\nExpected: {pid}\nActual: {response['tid']}"
        else:
            response = await self.recv_msg_of_connect_server()
            response = json.loads(response)
            assert response['type'] == 'addInstance', f"\nExpected: addInstance\nActual: {response['type']}"
            assert response['instanceId'] != 0, f"Worker instanceId can not be 0"
            assert response['tid'] != pid, f"Worker tid can not be {pid}"
            assert 'workerThread_' in response['name'], f"'workerThread_' not in {response['name']}"
        instance_id = await self.get_instance()
        send_queue = self.to_send_msg_queues[instance_id]
        recv_queue = self.received_msg_queues[instance_id]
        connection = ThreadConnectionInfo(instance_id, send_queue, recv_queue)
        return connection

    async def destroy_instance(self):
        response = await self.recv_msg_of_connect_server()
        response = json.loads(response)
        assert response['type'] == 'destroyInstance', f"\nExpected: destroyInstance\nActual: {response['type']}"
        return response
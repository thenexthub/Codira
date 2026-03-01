#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

#==================================================================
#文 件 名：                 TestAttachWorkerStep.py
#文件说明：                 多实例 attach 调试，执行单步操作进行跨线程通信
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  连接子线程 debugger server，用于执行 task 任务
    3.  所有线程使能 Runtime 和 Debugger
    4.  主线程 Index.ts 文件设置断点（Debugger.getPossibleAndSetBreakpointByUrl）
    5.  触发点击事件，主线程命中断点
    6.  子线程 Index.ts 文件设置断点（Debugger.getPossibleAndSetBreakpointByUrl）
    7.  子线程 resume，命中断点（Debugger.resume）
    8.  子线程 getProperties，返回给定对象的属性（Runtime.getProperties）
    9.  子线程 stepOut，主线程命中断点（Debugger.stepOut）
    10. 主线程 getProperties（Runtime.getProperties）
    11. 主线程 resume（Debugger.resume）
    11. 子线程命中断点后 resume（Debugger.resume）
    12. 关闭所有线程 debugger server 和 connect server 连接
#==================================================================
关键代码：
    Index.ets
        @Concurrent
        function add(args1, args2) {
            return args1 + args2
        }
        @Concurrent
        function sub(args1, args2) {
            return args1 - args2
        }
        let taskAdd = new taskpool.Task(add, 200, 100)
        let taskSub = new taskpool.Task(sub, 200, 100)
        async function taskpoolTest() {
            let valueAdd = await taskpool.execute(taskAdd)
            let valueSub = await taskpool.execute(taskSub)
        }
        .OnClick(() => {
            taskpoolTest()
        })
#!!================================================================
"""
import sys
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))  # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils
from cdp import debugger
from implement_api import debugger_api, runtime_api


class TestAttachWorkerStep(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '',
            'connect_server_port': 15610,
            'debugger_server_port': 15611,
            'bundle_name': 'com.example.multiWorker01',
            'hap_name': 'MultiWorker01.hap',
            'hap_path': str(resource_path / 'hap' / 'MultiWorker01.hap'),
            'file_path': {
                'entry_ability': 'entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ts',
                'index': 'entry|entry|1.0.0|src/main/ets/pages/Index.ts',
                'worker': 'entry|entry|1.0.0|src/main/ets/workers/Worker.ts'
            }
        }

    def setup(self):
        Step('1.下载应用')
        self.driver.install_app(self.config['hap_path'], "-r")
        Step('2.启动应用')
        self.driver.start_app(package_name=self.config['bundle_name'], params=self.config['start_mode'])
        self.config['pid'] = self.common_utils.get_pid(self.config['bundle_name'])
        assert self.config['pid'] != 0, f'Failed to get pid of {self.config["bundle_name"]}'
        Step('3.设置屏幕常亮')
        self.ui_utils.keep_awake()
        Step('4.端口映射，连接server')
        self.common_utils.connect_server(self.config)
        self.debugger_impl = debugger_api.DebuggerImpl(self.id_generator, self.config['websocket'])
        self.runtime_impl = runtime_api.RuntimeImpl(self.id_generator, self.config['websocket'])
        Step('5.attach调试应用')
        self.common_utils.attach(self.config['bundle_name'])

    def process(self):
        Step('6.执行测试用例')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception

    def teardown(self):
        Step('7.关闭应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('8.卸载应用')
        self.driver.uninstall_app(self.config['bundle_name'])

    async def test(self, websocket):
        ################################################################################################################
        # main thread: connect the debugger server
        ################################################################################################################
        main_thread = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], True)
        ################################################################################################################
        # worker thread: connect the debugger server
        ################################################################################################################
        worker_thread_1 = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], False)
        worker_thread_2 = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], False)
        ################################################################################################################
        # worker thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", worker_thread_1)
        await self.runtime_impl.send("Runtime.enable", worker_thread_2)
        ################################################################################################################
        # worker thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", worker_thread_1)
        await self.debugger_impl.send("Debugger.enable", worker_thread_2)
        ################################################################################################################
        # worker thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", worker_thread_1)
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", worker_thread_2)
        ################################################################################################################
        # main thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", main_thread)
        ################################################################################################################
        # main thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", main_thread)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread)
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=53),
                     debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=57)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        result_locations = response['result']['locations']
        self.common_utils.assert_equal(result_locations[0]['id'], 'id:53:0:' + self.config['file_path']['index'])
        self.common_utils.assert_equal(result_locations[1]['id'], 'id:57:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        ################################################################################################################
        # main thread: Debugger.paused, hit breakpoint
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:53:16:' + self.config['file_path']['index']])
        ################################################################################################################
        # worker thread: destroy instance
        ################################################################################################################
        response = await self.debugger_impl.destroy_instance()
        if response['instanceId'] == worker_thread_1.instance_id:
            worker_thread_1 = worker_thread_2
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread_1, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=11)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread_1, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:11:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused, hit breakpoint
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:57:20:' + self.config['file_path']['index']])
        ################################################################################################################
        # worker thread: connect the debugger server
        ################################################################################################################
        worker_thread_2 = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], False)
        ################################################################################################################
        # worker thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", worker_thread_2)
        ################################################################################################################
        # worker thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", worker_thread_2)
        ################################################################################################################
        # worker thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", worker_thread_2)
        # worker thread: Debugger.scriptParsed
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread_2)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread_2)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread_2, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=11)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread_2, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:11:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread_2)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:57:20:' + self.config['file_path']['index']])
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread_1)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread_1)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread_1)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:11:4:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.stepOut
        ################################################################################################################
        await self.debugger_impl.send("Debugger.stepOut", worker_thread_1)
        ################################################################################################################
        # worker thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", worker_thread_1)
        ################################################################################################################
        # main thread: Debugger.stepOver
        ################################################################################################################
        await self.debugger_impl.send("Debugger.stepOver", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread_2)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:11:4:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread_2)
        ################################################################################################################
        # worker thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", worker_thread_2)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # worker thread: destroy instance
        ################################################################################################################
        response = await self.debugger_impl.destroy_instance()
        assert response['instanceId'] != self.config['pid'], f"Worker instanceId can not be {self.config['pid']}"
        response = await self.debugger_impl.destroy_instance()
        assert response['instanceId'] != self.config['pid'], f"Worker instanceId can not be {self.config['pid']}"
        ################################################################################################################
        # main thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", main_thread)
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################
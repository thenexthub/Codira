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
#文 件 名：                 TestWorkerLaunchAccelerate.py
#文件说明：                 worker线程冷启性能优化测试
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger，开始运行
    3.  触发点击事件，连接子线程 debugger server
    4.  子线程使能 Runtime 和 Debugger
    5.  子线程设置所有断点（Debugger.saveAllPossibleBreakpoints）
    6.  子线程 scriptParsed 加载文件（Debugger.scriptPaused）
    7.  子线程多次 resume, 程序停在设置的断点处（Debugger.resume）
    8.  子线程在 params.ets 文件中设置断点（Debugger.getPossibleAndSetBreakpointsByUrl）
    9.  子线程 resume，命中断点（Debugger.resume）
    10. 子线程 resume（Debugger.resume）
    11. 关闭所有线程 debugger server 和 connect server 连接
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


class TestWorkerLaunchAccelerate(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15646,
            'debugger_server_port': 15647,
            'bundle_name': 'com.example.multiWorker09',
            'hap_name': 'MultiWorker09.hap',
            'hap_path': str(resource_path / 'hap' / 'MultiWorker09.hap'),
            'file_path': {
                'entry_ability': 'entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ts',
                'params': 'entry|entry|1.0.0|src/main/ets/resources/Params.ts',
                'index': 'entry|entry|1.0.0|src/main/ets/pages/Index.ts',
                'sendable': 'entry|entry|1.0.0|src/main/ets/resources/Sendable.ts',
                'person': 'entry|entry|1.0.0|src/main/ets/resources/Person.ts',
                'drop_frame': 'entry|entry|1.0.0|src/main/ets/resources/DropFrame.ts',
                'step': 'entry|entry|1.0.0|src/main/ets/resources/Step.ts',
                'calculate': 'entry|entry|1.0.0|src/main/ets/resources/Calculate.ts',
                'sleep': 'entry|entry|1.0.0|src/main/ets/resources/Sleep.ts',
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

    def process(self):
        Step('5.执行测试用例')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception

    def teardown(self):
        Step('6.关闭应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('7.卸载应用')
        self.driver.uninstall_app(self.config['bundle_name'])

    async def test(self, websocket):
        ################################################################################################################
        # main thread: connect the debugger server
        ################################################################################################################-
        main_thread = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], True)
        ################################################################################################################
        # main thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", main_thread)
        ################################################################################################################
        # main thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", main_thread, debugger.EnableAccelerateLaunchParams())
        ################################################################################################################
        # main thread: Debugger.saveAllPossibleBreakpoints
        ################################################################################################################
        params = debugger.SaveAllPossibleBreakpointsParams({})
        await self.debugger_impl.send("Debugger.saveAllPossibleBreakpoints", main_thread, params)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['entry_ability'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['sendable'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['person'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['params'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['drop_frame'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['calculate'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['step'])
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['index'])
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        ################################################################################################################
        # worker thread: connect the debugger server
        ################################################################################################################-
        worker_thread = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], False)
        ################################################################################################################
        # worker thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", worker_thread, debugger.EnableAccelerateLaunchParams())
        ################################################################################################################
        # worker thread: Debugger.saveAllPossibleBreakpoints
        ################################################################################################################
        locations = {
            self.config['file_path']['step']: [3],
            self.config['file_path']['worker']: [15, 17, 1000],
            self.config['file_path']['params']: [77, 79, 83]
        }
        params = debugger.SaveAllPossibleBreakpointsParams(locations)
        await self.debugger_impl.send("Debugger.saveAllPossibleBreakpoints", worker_thread, params)
        ################################################################################################################
        # worker thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['sendable'])
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['person'])
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['params'])
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['drop_frame'])
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['calculate'])
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['step'])
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:15:4:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:3:19:' + self.config['file_path']['step']])
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['step'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:17:4:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:77:4:' + self.config['file_path']['params']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:79:4:' + self.config['file_path']['params']])
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['params'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['params'], line_number=84)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        result_locations = response['result']['locations']
        self.common_utils.assert_equal(result_locations[0]['id'], 'id:84:0:' + self.config['file_path']['params'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:84:4:' + self.config['file_path']['params']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", worker_thread)
        ################################################################################################################
        # main thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", main_thread)
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(worker_thread.instance_id, worker_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################
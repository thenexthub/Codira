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
#文 件 名：                 TestMainBreakpoints.py
#文件说明：                 主线程 debug 调试之 drop frame
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  创建子线程用于执行 task，连接子线程 debugger server
    4.  子线程使能 Runtime 和 Debugger
    5.  主线程 Index.ets 文件设置断点（Debugger.getPossibleAndSetBreakpointByUrl）
    6.  主线程 resume，停在断点处（Debugger.resume）
    7.  主线程 evaluateOnCallFrame，观察指定变量的值（Debugger.evaluateOnCallFrame）
    8.  主线程时光调试，回到方法调用前（Debugger.dropFrame）
    9.  主线程 evaluateOnCallFrame，观察指定变量的值是否变化（Debugger.evaluateO*nCallFrame）
    10. 主线程重复步骤 6-10，测试 dropFrame 在不同方法内的执行情况
    11. 执行到 taskpool 任务时切换到子线程进行 dropFrame 操作（Debugger.dropFrame）
    12. 所有线程去使能 debugger（Debugger.disable）
    13. 关闭所有线程 debugger server 和 connect server 连接
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


class TestMainDropFrame(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15604,
            'debugger_server_port': 15605,
            'bundle_name': 'com.example.mainInstance01',
            'hap_name': 'MainInstance01.hap',
            'hap_path': str(resource_path / 'hap' / 'MainInstance01.hap'),
            'file_path': {
                'entry_ability': 'entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ts',
                'index': 'entry|entry|1.0.0|src/main/ets/pages/Index.ts',
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
        ################################################################################################################
        main_thread = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], True)
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
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['entry_ability'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        ################################################################################################################
        # main thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['entry_ability'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        ################################################################################################################
        # main thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # worker thread: connect the debugger server
        ################################################################################################################
        worker_thread = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], False)
        ################################################################################################################
        # worker thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", worker_thread)
        ################################################################################################################
        # worker thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", worker_thread)
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=79)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:79:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:79:9:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        expected_result = {"type": "number", "unserializableValue": "0", "description": "0"}
        self.common_utils.assert_equal(response['result']['result'], expected_result)
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        expected_result = {"type": "number", "unserializableValue": "0", "description": "0"}
        self.common_utils.assert_equal(response['result']['result'], expected_result)
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=91)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:91:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:91:5:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=105)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:105:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:105:5:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:105:5:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "24", "description": "24"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=119)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:119:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:119:13:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:119:13:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "26", "description": "26"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "12", "description": "12"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=134)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:134:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:134:13:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('s')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('func')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        function_info = response['result']['result']
        self.common_utils.assert_equal(function_info['type'], 'function')
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('s')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('func')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], function_info['description'])
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('s')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        params = debugger.EvaluateOnCallFrameParams('func')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        assert response['result']['result']['description'] != function_info['description']
        self.common_utils.assert_equal(response['result']['result']['type'], 'function')
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=151)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:151:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:151:1:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "11", "description": "11"})
        params = debugger.EvaluateOnCallFrameParams('x')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "8", "description": "8"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "9", "description": "9"})
        params = debugger.EvaluateOnCallFrameParams('x')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "7", "description": "7"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "8", "description": "8"})
        params = debugger.EvaluateOnCallFrameParams('x')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "7", "description": "7"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=161)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:161:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:161:5:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "12", "description": "12"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=175)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:175:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:175:13:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "5", "description": "5"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "12", "description": "12"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=186)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:186:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:186:13:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "9", "description": "9"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "22", "description": "22"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "5", "description": "5"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "5", "description": "5"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=204)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:204:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:204:13:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "14", "description": "14"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "21", "description": "21"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=220)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:220:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:220:9:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "17", "description": "17"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "14", "description": "14"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=236)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:236:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:236:5:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        P1_INTRODUCE = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAAoQAAANQAAACAAAAAZAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////oQAAANQAAACAAAAAkgAAAJ0AAAAhZGVidWdnZXJHZXRWYWx1ZQATaW50cm9kdWNlAAVw'
            'MQAzTF9FU1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP//xw'
            'AAAIgCAR4BAAACAAVWAQAABhEBAAAAFVNsb3ROdW1iZXIAAAABAAUBAAAIAAAANwoDLwBEoESxRMJtYQZgBkIAAABhBz4CAGEIAmEJYAcr'
            'AggJYQVgBUIEAQBhBGAELQYFZAtrAQ8A/////w8AAgAoAFEBAAA=')
        ARR_JOIN = (
            'UEFOREEAAAAAAAAADAACAGABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFwBAAAAAAAARAAAAAEAAABEAAAAnQAAANAAAACAAAAAYAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////nQAAANAAAACAAAAAhQAAAJcAAAAHYXJyACFkZWJ1Z2dlckdldFZhbHVlAAlqb2luADNM'
            'X0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAAEA///DAAAAiA'
            'IBGgEAAAIABVIBAAAGDQEAAAAVU2xvdE51bWJlcgAAAAEAAQEAAAgAAAA3CgMvAESgRLFEwm1hBmAGQgABAGEHPgAAYQgCYQlgBysCCAlh'
            'BWAFQgQCAGEEYAQtBgVkC2sBDwD/////DwACACgATQEAAA==')
        params = debugger.EvaluateOnCallFrameParams(P1_INTRODUCE)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "name = Kate; age = 10")
        params = debugger.EvaluateOnCallFrameParams('map')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        assert response['result']['result']['description'].startswith("Map(2) {0 => 'str0', 1 => 'str1'}")
        params = debugger.EvaluateOnCallFrameParams(ARR_JOIN)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "10,11,2,3")
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams(P1_INTRODUCE)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "name = Kate; age = 10")
        params = debugger.EvaluateOnCallFrameParams('map')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        assert response['result']['result']['description'].startswith("Map(2) {0 => 'str0', 1 => 'str1'}")
        params = debugger.EvaluateOnCallFrameParams(ARR_JOIN)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "10,11,2,3")
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=244)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:244:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:244:5:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams(P1_INTRODUCE)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "name = Tony; age = 30")
        params = debugger.EvaluateOnCallFrameParams('map')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        assert response['result']['result']['description'].startswith("Map(1) {2 => 'str2'}")
        params = debugger.EvaluateOnCallFrameParams(ARR_JOIN)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "4,3,2,1,0")
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams(P1_INTRODUCE)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "name = Kate; age = 10")
        params = debugger.EvaluateOnCallFrameParams('map')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        assert response['result']['result']['description'].startswith("Map(2) {0 => 'str0', 1 => 'str1'}")
        params = debugger.EvaluateOnCallFrameParams(ARR_JOIN)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result']['description'], "10,11,2,3")
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=251)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:251:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:251:19:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=266)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:266:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:266:22:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=272)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:272:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:272:8:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
        # main thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=29)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:29:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:29:9:' + self.config['file_path']['index']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('ac')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "20", "description": "20"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('ac')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        response = await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['message'], 'Not yet support sendable method')
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=283)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:283:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:283:9:' + self.config['file_path']['index']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('ia')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "200", "description": "200"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('ia')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "120", "description": "120"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('ia')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "100", "description": "100"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
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
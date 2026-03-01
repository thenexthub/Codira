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
#文 件 名：                 TestWorkerDropFrame01.py
#文件说明：                 多实例 debug 调试之 drop frame
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程 resume（Debugger.resume）
    4.  触发点击事件，创建子线程，连接子线程 debugger server
    5.  子线程使能 Runtime 和 Debugger
    6.  子线程 Worker.ets 文件设置断点（Debugger.getPossibleAndSetBreakpointByUrl）
    7.  子线程 resume，停在断点处（Debugger.resume）
    8.  子线程 evaluateOnCallFrame，观察指定变量的值（Debugger.evaluateOnCallFrame）
    9.  子线程时光调试，回到方法调用前（Debugger.dropFrame）
    10. 子线程 evaluateOnCallFrame，观察指定变量的值是否变化（Debugger.evaluateOnCallFrame）
    11. 子线程重复步骤 6-10，测试 dropFrame 在不同方法内的执行情况
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


class TestWorkerDropFrame01(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15634,
            'debugger_server_port': 15635,
            'bundle_name': 'com.example.multiWorker08',
            'hap_name': 'MultiWorker08.hap',
            'hap_path': str(resource_path / 'hap' / 'MultiWorker08.hap'),
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
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
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
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=64)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:64:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:64:9:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('b')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('c')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=76)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:76:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:76:5:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=90)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:90:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:90:5:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:90:5:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "24", "description": "24"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=104)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:104:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:104:13:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:104:13:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "26", "description": "26"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "12", "description": "12"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=121)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:121:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:121:9:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('s')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('func')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        function_info = response['result']['result']
        self.common_utils.assert_equal(function_info['type'], 'function')
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('s')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        params = debugger.EvaluateOnCallFrameParams('func')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        assert response['result']['result']['description'] != function_info['description'], \
            f'The current description should not be the same as before.'
        self.common_utils.assert_equal(response['result']['result']['type'], 'function')
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=136)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:136:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:136:1:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "11", "description": "11"})
        params = debugger.EvaluateOnCallFrameParams('x')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "8", "description": "8"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "9", "description": "9"})
        params = debugger.EvaluateOnCallFrameParams('x')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "7", "description": "7"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "8", "description": "8"})
        params = debugger.EvaluateOnCallFrameParams('x')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "7", "description": "7"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=146)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:146:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:146:5:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "12", "description": "12"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=160)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:160:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:160:13:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "5", "description": "5"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "12", "description": "12"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=171)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:171:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:171:13:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "9", "description": "9"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "22", "description": "22"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "5", "description": "5"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "5", "description": "5"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=189)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:189:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:189:13:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "14", "description": "14"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "21", "description": "21"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "1", "description": "1"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "10", "description": "10"})
        params = debugger.EvaluateOnCallFrameParams('e')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], "undefined")
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=205)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:205:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:205:9:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "3", "description": "3"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "17", "description": "17"})
        ################################################################################################################
        # worker thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send("Debugger.dropFrame", worker_thread, params)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'], [])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('a')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        params = debugger.EvaluateOnCallFrameParams('d')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "14", "description": "14"})
        ################################################################################################################
        # worker thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['worker'])
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
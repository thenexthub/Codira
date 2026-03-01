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
#文 件 名：                 TestArkTSDyn2StaStep.py
#文件说明：                 动态类型到静态类型的单步调试
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程运行等到的调试器
    4.  主线程解析 entry_ability 和 index 文件
    5.  主线程 resume 验证是否正确命中1.2文件 export_static
    6.  主线程发起两个taskpool协程
    7.  两个协程分别能step into，paused，runtime.getproperties
    8.  主线程resume放行所有协程
    9.  主线程正确命中1.1文件index
    10. 关闭主线程debugger server和connect server连接
#==================================================================
"""
import sys
from pathlib import Path
import time
root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))  # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils
from cdp import debugger, runtime
from implement_api import debugger_api, runtime_api


class TestArkTSDyn2StaStep(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15654,
            'debugger_server_port': 15655,
            'bundle_name': 'com.example.Interop1use2',
            'hap_name': 'Interop1use2.hap',
            'hap_path': str(resource_path / 'hap' / 'Interop1use2.hap'),
            'file_path': {
                'entry_ability': 'entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ts',
                'index': 'entry|entry|1.0.0|src/main/ets/pages/Index.ts',
                'export_static': 'har_2\src\main\ets\components\sts_2_ets\export_static.ets',
                'export_static_bridge': 'entry|har_2|1.0.0|build/default/intermediates/declgen/default/' + 
                                        'declgenBridgeCode/har_2/src/main/ets/components/sts_2_ets/export_static.ts',
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
        main_thread = await websocket.connect_to_debugger_server(self.config['pid'], True, True)
        ################################################################################################################
        # main thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", main_thread)
        await self.runtime_impl.send("Runtime.enableStatic", main_thread, None, True, 2)
        ################################################################################################################
        # main thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", main_thread)
        await self.debugger_impl.send("Debugger.enableStatic", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['entry_ability'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['entry_ability'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread, None)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['export_static_bridge'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['export_static_bridge'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread, None)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['export_static'])
        self.common_utils.assert_equal(response['params']['endLine'], 2147483647)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['export_static'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['export_static'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['export_static'], line_number=53),
                     debugger.BreakLocationUrl(url=self.config['file_path']['export_static'], line_number=54),
                     debugger.BreakLocationUrl(url=self.config['file_path']['export_static'], line_number=29),
                     debugger.BreakLocationUrl(url=self.config['file_path']['export_static'], line_number=36)]
        params = debugger.SetBreakpointsLocations(locations)
        await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resumeStatic", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: 1.1 resume, paused在1.2
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread, None)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: 1.2 resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resumeStatic", main_thread, None, True, 1)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'other')  ## other
        ################################################################################################################
        # main thread: Debugger.resumeStatic 当前在1.2文件处，所以1.2resume，paused命中2个协程
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resumeStatic", main_thread, None, True, 1)
        # {"id": 242, "method": "Debugger.resume", "params": {}}
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        # self.common_utils.assert_equal(response['sessionId'], '12')  ## sessionId is 12
        sessionId1 = response['sessionId']
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        # self.common_utils.assert_equal(response['sessionId'], '20')  ## sessionId is 20
        sessionId2 = response['sessionId']
        ################################################################################################################
        # sessionId1: Debugger.stepIntoStatic
        ################################################################################################################
        await self.debugger_impl.send("Debugger.stepIntoStatic", main_thread, None, True, 2, sessionId1)
        ################################################################################################################
        # sessionId1: Debugger.stepIntoStatic
        ################################################################################################################
        await self.debugger_impl.send("Debugger.stepIntoStatic", main_thread, None, True, 2, sessionId1)
        time.sleep(2)
        ################################################################################################################
        # sessionId1: Runtime.getPropertiesStatic
        ################################################################################################################
        params = runtime.GetPropertiesParams('0')
        await self.runtime_impl.send("Runtime.getPropertiesStatic", main_thread, None, True, 1, sessionId1)
        ################################################################################################################
        # sessionId2: Debugger.stepIntoStatic
        ################################################################################################################
        await self.debugger_impl.send("Debugger.stepIntoStatic", main_thread, None, True, 2, sessionId2)
        ################################################################################################################
        # sessionId2: Debugger.stepIntoStatic
        ################################################################################################################
        await self.debugger_impl.send("Debugger.stepIntoStatic", main_thread, None, True, 2, sessionId2)
        time.sleep(2)
        ################################################################################################################
        # sessionId2: Runtime.getPropertiesStatic
        ################################################################################################################
        params = runtime.GetPropertiesParams('0')
        await self.runtime_impl.send("Runtime.getPropertiesStatic", main_thread, None, True, 1, sessionId2)
        ################################################################################################################
        # resume all sessions
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resumeStatic", main_thread, None, True, 2)
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################
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
#文 件 名：                 TestWorkerEvaluateOnCallFrame.py
#文件说明：                 多实例 debug 调试 evaluateOnCallFrame
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程 Index.ts 文件设置断点（Debugger.getPossibleAndSetBreakpointByUrl）
    4.  主线程 resume，暂停在下一断点（Debugger.resume）
    5.  创建子线程，连接子线程 debugger server
    6.  子线程使能 Runtime 和 Debugger
    7.  子线程 Worker.ets 文件设置断点（Debugger.getPossibleAndSetBreakpointByUrl）
    8.  主线程 resume，发送消息给子线程，子线程暂停在断点（Debugger.resume）
    9.  子线程对 local/closure/module/global 作用域变量进行查看、修改等操作（Debugger.evaluateOnCallFrame）
    10. 触发点击事件，销毁子线程，对应的 debugger server 连接断开
    11. 关闭主线程 debugger server 和 connect server 连接
#==================================================================
关键代码：
    Index.ets
        let myWorker = new worker.ThreadWorker("entry/ets/workers/Worker.ets")
        myWorker.postMessage("hello world")
        .OnClick(() => {
            myWorker.terminate()
        })
    Worker.ets
        const workerPort: ThreadWorkerGlobalScope = worker.workerPort;
        let closureBoolean = false
        ...... // 定义不同的闭包变量
        globalThis.globalBool = new Boolean(closureBoolean)
        ...... // 定义不同的全局变量
        workerPort.onmessage = (e: MessageEvents) => {
            let localNull = null
            ...... // 定义不同的局部变量
            workerPort.postMessage(e.data)
        }
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


class TestWorkerEvaluateOnCallFrame(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15638,
            'debugger_server_port': 15639,
            'bundle_name': 'com.example.multiWorker04',
            'hap_name': 'MultiWorker04.hap',
            'hap_path': str(resource_path / 'hap' / 'MultiWorker04.hap'),
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
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['index'], line_number=12)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:12:0:' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused, hit breakpoint
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:12:0:' + self.config['file_path']['index']])
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
        # worker thread: Debugger.scriptParsed
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
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['worker'], line_number=116)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", worker_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:116:0:' + self.config['file_path']['worker'])
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        # worker thread: Debugger.paused
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:116:4:' + self.config['file_path']['worker']])
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        LOCALPERSON_AGE = (
            'UEFOREEAAAAAAAAADAACAGABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFwBAAAAAAAARAAAAAEAAABEAAAApAAAANcAAACAAAAAYAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pAAAANcAAACAAAAAhQAAAJcAAAAHYWdlACFkZWJ1Z2dlckdldFZhbHVlABdsb2NhbFBl'
            'cnNvbgAzTF9FU1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP'
            '//ygAAAIgCASEBAAACAAVSAQAABhQBAAAAFVNsb3ROdW1iZXIAAAABAAgBAAAGAAAANwkDKABEkEShRLJtYQVgBUIAAQBhBj4CAGEHAmEI'
            'YAYrAgcIYQRgBEIEAABkC2sBDwD/////DwACACEATQEAAA==')
        params = debugger.EvaluateOnCallFrameParams(LOCALPERSON_AGE)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "20", "description": "20"})
        SET_LOCALPERSON_AGE_22 = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAApAAAANcAAACAAAAAZAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pAAAANcAAACAAAAAhQAAAJcAAAAHYWdlACFkZWJ1Z2dlckdldFZhbHVlABdsb2NhbFBl'
            'cnNvbgAzTF9FU1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP'
            '//ygAAAIgCASEBAAACAAVWAQAABhQBAAAAFVNsb3ROdW1iZXIAAAABAAgBAAAGAAAANwkDLABEkEShRLJtYQVgBUIAAQBhBj4CAGEHAmEI'
            'YAYrAgcIYQRiFgAAAEMEAAAEZAtrAQ8A/////w8AAgAlAFEBAAA=')
        params = debugger.EvaluateOnCallFrameParams(SET_LOCALPERSON_AGE_22)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "22", "description": "22"})
        LOCALPERSON_AGE_PLUS_10 = (
            'UEFOREEAAAAAAAAADAACAGwBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGgBAAAAAAAARAAAAAEAAABEAAAApAAAANcAAACAAAAAbAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pAAAANcAAACAAAAAhQAAAJcAAAAHYWdlACFkZWJ1Z2dlckdldFZhbHVlABdsb2NhbFBl'
            'cnNvbgAzTF9FU1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP'
            '//ygAAAIgCASEBAAACAAVcAQAABhQBAAAAFVNsb3ROdW1iZXIAAAABAAgBAAAHAAAANwoDMgBEoESxRMJtYQZgBkIAAQBhBz4CAGEIAmEJ'
            'YAcrAggJYQVgBUIEAABhBGIKAAAACgYEZAtrAQ8A/////w8AAgArAAAAVwEAAA==')
        params = debugger.EvaluateOnCallFrameParams(LOCALPERSON_AGE_PLUS_10)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "32", "description": "32"})
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        SET_LOCALPERSON_NAME_TIM_AGE_30 = (
            'UEFOREEAAAAAAAAADAACAKgBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAKQBAAAAAAAARAAAAAEAAABEAAAAxgAAAPkAAACIAAAAqAEAAAIAAA'
            'BsAAAABQAAAHQAAAD/////////////////////xgAAAPkAAACIAAAAkAAAAJUAAACnAAAAuQAAAA1QZXJzb24AB1RpbQAhZGVidWdnZXJH'
            'ZXRWYWx1ZQAhZGVidWdnZXJTZXRWYWx1ZQAXbG9jYWxQZXJzb24AM0xfRVNTbG90TnVtYmVyQW5ub3RhdGlvbjsAAAAAAIFAAAACAAAXZn'
            'VuY19tYWluXzAAE0xfR0xPQkFMOwAAAAAAAQABAgAAAQD//+wAAACIAgFDAQAAAgAFmAEAAAY2AQAAABVTbG90TnVtYmVyAAAAAQAqAQAA'
            'CgAAADcJA0wARJBEoUSybWEFYAVCAAIAYQY+AABhBwJhCGAGKwIHCGEEPgEAYQViHgAAAGEGCAQDBGEEbWEFYAVCBgMAYQY+BABhB0RIYA'
            'YrCAcIZAtrAQ8A/////w8AAgBFAAAAkwEAAA=='
        )
        params = debugger.EvaluateOnCallFrameParams(SET_LOCALPERSON_NAME_TIM_AGE_30)
        await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        LOCALPERSON_NAME = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAApQAAANgAAACAAAAAZAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pQAAANgAAACAAAAAkgAAAJ8AAAAhZGVidWdnZXJHZXRWYWx1ZQAXbG9jYWxQZXJzb24A'
            'CW5hbWUAM0xfRVNTbG90TnVtYmVyQW5ub3RhdGlvbjsAAAAAAIFAAAACAAAXZnVuY19tYWluXzAAE0xfR0xPQkFMOwAAAAAAAQABAgAAAQ'
            'D//8sAAACIAgEiAQAAAgAFUwEAAAYVAQAAABVTbG90TnVtYmVyAAAAAQAJAQAABgAAADcJAygARJBEoUSybWEFYAVCAAAAYQY+AQBhBwJh'
            'CGAGKwIHCGEEYARCBAIAZAtrAQ8A/////w8AAgAhAAAAAE4BAAA=')
        params = debugger.EvaluateOnCallFrameParams(LOCALPERSON_NAME)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "string", "unserializableValue": "Tim", "description": "Tim"})
        params = debugger.EvaluateOnCallFrameParams(LOCALPERSON_AGE)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "30", "description": "30"})
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('closureString')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "string", "unserializableValue": "closure", "description": "closure"})
        params = debugger.EvaluateOnCallFrameParams('closureString = "modified"')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "string", "unserializableValue": "modified", "description": "modified"})
        params = debugger.EvaluateOnCallFrameParams('closureString')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "string", "unserializableValue": "modified", "description": "modified"})
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        CLOSUREMAP_GET_1 = (
            'UEFOREEAAAAAAAAADAACAHABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGwBAAAAAAAARAAAAAEAAABEAAAAowAAANYAAACAAAAAcAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////owAAANYAAACAAAAAjAAAAJ4AAAAVY2xvc3VyZU1hcAAhZGVidWdnZXJHZXRWYWx1ZQAH'
            'Z2V0ADNMX0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAAEA//'
            '/JAAAAiAIBIAEAAAIABWABAAAGEwEAAAAVU2xvdE51bWJlcgAAAAEABwEAAAgAAAA3CgM3AESgRLFEwm1hBmAGQgABAGEHPgAAYQgCYQlg'
            'BysCCAlhBWAFQgQCAGEEYgEAAABhBmAELgYFBmQLawEPAP////8PAAIAMAAAAFsBAAA=')
        params = debugger.EvaluateOnCallFrameParams(CLOSUREMAP_GET_1)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'], {"type": "undefined"})
        CLOSUREMAP_SET_1_2 = (
            'UEFOREEAAAAAAAAADAACAHgBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAHQBAAAAAAAARAAAAAEAAABEAAAAowAAANYAAACAAAAAeAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////owAAANYAAACAAAAAjAAAAJ4AAAAVY2xvc3VyZU1hcAAhZGVidWdnZXJHZXRWYWx1ZQAH'
            'c2V0ADNMX0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAAEA//'
            '/JAAAAiAIBIAEAAAIABWgBAAAGEwEAAAAVU2xvdE51bWJlcgAAAAEABwEAAAgAAAA3CgM/AESgRLFEwm1hBmAGQgABAGEHPgAAYQgCYQlg'
            'BysCCAlhBWAFQgQCAGEEYgEAAABhBmICAAAAYQdgBC8GBQYHZAtrAQ8A/////w8AAgA4AAAAYwEAAA==')
        params = debugger.EvaluateOnCallFrameParams(CLOSUREMAP_SET_1_2)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        assert response['result']['result']['description'].startswith('Map(1) {1 => 2}'), \
            f'The description of the evaluation "closureMap.set(1, 2)" is incorrect.'
        params = debugger.EvaluateOnCallFrameParams(CLOSUREMAP_GET_1)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "2", "description": "2"})
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('globalArray')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        assert response['result']['result']['description'].startswith('Array(3)'), \
            f'The description of the parameter globalArray is incorrect.'
        GLOBALARRAY_PUSH_999 = (
            'UEFOREEAAAAAAAAADAACAHABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGwBAAAAAAAARAAAAAEAAABEAAAApQAAANgAAACAAAAAcAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pQAAANgAAACAAAAAkgAAAJ8AAAAhZGVidWdnZXJHZXRWYWx1ZQAXZ2xvYmFsQXJyYXkA'
            'CXB1c2gAM0xfRVNTbG90TnVtYmVyQW5ub3RhdGlvbjsAAAAAAIFAAAACAAAXZnVuY19tYWluXzAAE0xfR0xPQkFMOwAAAAAAAQABAgAAAQ'
            'D//8sAAACIAgEiAQAAAgAFYgEAAAYVAQAAABVTbG90TnVtYmVyAAAAAQAJAQAACAAAADcKAzcARKBEsUTCbWEGYAZCAAAAYQc+AQBhCAJh'
            'CWAHKwIICWEFYAVCBAIAYQRi5wMAAGEGYAQuBgUGZAtrAQ8A/////w8AAgAwAF0BAAA=')
        params = debugger.EvaluateOnCallFrameParams(GLOBALARRAY_PUSH_999)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "4", "description": "4"})
        GLOBALARRAY_INDEX_3 = (
            'UEFOREEAAAAAAAAADAACAGABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFwBAAAAAAAARAAAAAEAAABEAAAAmwAAAM4AAAB8AAAAYAEAAAIAAA'
            'BsAAAAAgAAAHQAAAD/////////////////////mwAAAM4AAAB8AAAAjgAAACFkZWJ1Z2dlckdldFZhbHVlABdnbG9iYWxBcnJheQAzTF9F'
            'U1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP//wQAAAIgCAR'
            'gBAAACAAVPAQAABgsBAAAAFVNsb3ROdW1iZXIAAAABAP8AAAAGAAAANwkDLgBEkEShRLJtYQVgBUIAAABhBj4BAGEHAmEIYAYrAgcIYQRi'
            'AwAAAGEFYAU3BARkC2sBDwD/////DwACACcAAAAASgEAAA==')
        params = debugger.EvaluateOnCallFrameParams(GLOBALARRAY_INDEX_3)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "999", "description": "999"})
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('globalBool')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], 'object')
        params = debugger.EvaluateOnCallFrameParams('globalBool = true')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "boolean", "unserializableValue": "true", "description": "true"})
        params = debugger.EvaluateOnCallFrameParams('globalBool')
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "boolean", "unserializableValue": "true", "description": "true"})
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        WORKER_WORKERPORT = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAApgAAANkAAACAAAAAZAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pgAAANkAAACAAAAAkgAAAJoAAAAhZGVidWdnZXJHZXRWYWx1ZQANd29ya2VyABV3b3Jr'
            'ZXJQb3J0ADNMX0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAA'
            'EA///MAAAAiAIBIwEAAAIABVQBAAAGFgEAAAAVU2xvdE51bWJlcgAAAAEACgEAAAYAAAA3CQMoAESQRKFEsm1hBWAFQgAAAGEGPgEAYQcC'
            'YQhgBisCBwhhBGAEQgQCAGQLawEPAP////8PAAIAIQAAAE8BAAA=')
        params = debugger.EvaluateOnCallFrameParams(WORKER_WORKERPORT)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], 'object')
        SET_WORKER_WORKERPORT_UNDEFINED = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAApgAAANkAAACAAAAAZAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pgAAANkAAACAAAAAkgAAAJoAAAAhZGVidWdnZXJHZXRWYWx1ZQANd29ya2VyABV3b3Jr'
            'ZXJQb3J0ADNMX0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAA'
            'EA///MAAAAiAIBIwEAAAIABVQBAAAGFgEAAAAVU2xvdE51bWJlcgAAAAEACgEAAAYAAAA3CQMoAESQRKFEsm1hBWAFQgAAAGEGPgEAYQcC'
            'YQhgBisCBwhhBABDBAIABGQLawEPAP////8PAAIAIQAAAE8BAAA=')
        params = debugger.EvaluateOnCallFrameParams(SET_WORKER_WORKERPORT_UNDEFINED)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], 'undefined')
        params = debugger.EvaluateOnCallFrameParams(WORKER_WORKERPORT)
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], 'undefined')
        ################################################################################################################
        # worker thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams("Deque")
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result']['type'], 'function')
        params = debugger.EvaluateOnCallFrameParams("Deque = 0")
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        params = debugger.EvaluateOnCallFrameParams("Deque")
        response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", worker_thread, params)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        ################################################################################################################
        # worker thread: destroy instance
        ################################################################################################################
        response = await self.debugger_impl.destroy_instance()
        self.common_utils.assert_equal(response['instanceId'], worker_thread.instance_id)
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
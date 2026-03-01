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
#文 件 名：                 TestConcurrentSymbolicBreakpoints.py
#文件说明：                 验证taskpool场景下的符号断点功能
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  连接子线程 debugger server，用于执行task任务
    4.  子线程使能 Runtime 和 Debugger
    5.  子线程 Index.ts文件为add方法设置符号断点(Debugger.setSymbolicBreakpoints)
    6.  触发点击事件
    7.  子线程加载 Index.ts(Debugger.scriptParsed)
    8.  子线程暂停
    9.  子线程 resume(Debugger.resume)
    10. 子线程命中符号断点add(Debugger.paused)
    11. assert符号断点位置，是否处于add方法首行
    12. 子线程 resume(Debugger.resume)
    13. 子线程移除add方法符号断点(Debugger.removeSymbolicBreakpoints)
    14. 子线程 Index.ts文件为sub 方法设置符号断点(Debugger.setSymbolicBreakpoints)
    15. 触发点击事件
    16. 子线程命中符号断点sub(Debugger.paused)
    17. assert符号断点位置，是否处于sub方法首行
    18. 子线程 resume(Debugger.resume)
    19. 所有线程disable debugger
    20. 关闭主线程debugger server和connect server连接
#==================================================================
关键代码：
    Index.ets
        @Concurrent
        function add(args1, args2) {
            return args1 + args2;
        }
        @Concurrent
        function sub(args1, args2) {
            return args1 - args2;
        }
        let taskAdd = new taskpool.Task(add, 200, 100);
        let taskSub = new taskpool.Task(add, 200, 100);
        async function taskpoolTest() {
            let valueAdd = await taskpool.execute(taskAdd);
            let valueSdd = await taskpool.execute(taskSub);
        }
        .OnClick(() => {
            taskpoolTest();
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
from devicetest.log.logger import DeviceTestLog as log


class TestConcurrentSymbolicBreakpoints(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '',
            'connect_server_port': 15654,
            'debugger_server_port': 15655,
            'bundle_name': 'com.example.taskPool01',
            'hap_name': 'TaskPool01.hap',
            'hap_path': str(resource_path / 'hap' / 'TaskPool01.hap'),
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
        main_thread = await websocket.connect_to_debugger_server(self.config['pid'], True)
        ################################################################################################################
        # worker thread: connect the debugger server
        ################################################################################################################
        worker_thread = await websocket.connect_to_debugger_server(self.config['pid'], False)
        ################################################################################################################
        # main thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", main_thread)
        ################################################################################################################
        # worker thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", worker_thread)
        ################################################################################################################
        # main thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", main_thread)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread)
        ################################################################################################################
        # worker thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", worker_thread)
        ################################################################################################################
        # worker thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoints(functionName='add')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", worker_thread, params)
        ################################################################################################################
        # click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        ################################################################################################################
        # worker thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", worker_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['endLine'], 0)
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused, hit symbolic breakpoint, function name is add
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "add")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 8)
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.removeSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoints(functionName='add')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.removeSymbolicBreakpoints", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoints(functionName='sub')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", worker_thread, params)
        ################################################################################################################
        # click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        ################################################################################################################
        # worker thread: Debugger.paused, hit symbolic breakpoint, function name is sub
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "sub")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 15)
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
# !/usr/bin/env python
# coding: utf-8
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
#文 件 名：                 TestWorkerSymbolicBreakpoints.py
#文件说明：                 多实例 debug 调试 SymbolicBreakpoints
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程 resume（Debugger.resume）
    4.  主线程 Index.ets 文件设置符号断点断点 函数add（Debugger.setSymbolicBreakpoints）
    5.  主线程 命中符号断点 函数add （Debugger.paused）
    6.  主线程 Index.ets 文件设置符号断点断点 函数sub（Debugger.setSymbolicBreakpoints）
    7.  主线程 命中符号断点 函数sub （Debugger.paused）
    8.  触发点击事件，创建子线程，连接子线程 debugger server
    9.  子线程使能 Runtime 和 Debugger
    10. 子线程 resume，停在断点处（Debugger.resume）
    11. 子线程 命中符号断点 函数add （Debugger.paused）
    12. 子线程 Worker.ets 文件设置符号断点断点 函数setAge（Debugger.setSymbolicBreakpoints）
    13. 子线程 Worker.ets 文件删除符号断点断点 函数add（Debugger.removeSymbolicBreakpoints）
    14. 子线程 resume，继续执行（Debugger.resume）
    15. 子线程 命中符号断点 函数sub （Debugger.paused）
    16. 子线程 resume，继续执行（Debugger.resume）
    17. 子线程 命中符号断点 函数setAge （Debugger.paused）
    18. 子线程 resume，继续执行（Debugger.resume）
    19. 所有线程去使能 debugger（Debugger.disable）
    20. 关闭所有线程 debugger server 和 connect server 连接
#==================================================================
关键代码：
    Index.ets
        .OnClick(() => {
            let myWorker = new worker.ThreadWorker("entry/ets/workers/Worker.ets")
            let age = add(10, 20, 30)
            age = sub(age)
            myWorker.onmessage = (e: MessageEvents) => {}
            myWorker.postMessage("hello world")
        })
        function add(num1: number, num2: number, num3 : number):number {
            // 测试add同名函数
            let result = num1 + num2
            return result
        }
        function sub(num1: number):number {
            let result = num1 - 10
            return result
        }
    Worker.ets
        const workerPort: ThreadWorkerGlobalScope = worker.workerPort;
        workerPort.onmessage = (e: MessageEvents) => {
            let person = new Person('Tim', add(10, 11))
            person.introduce()
            let age = sub(4, 1) + add(3, 6)
            workerPort.postMessage(e.data)
        }
        function add(args1, args2) {
            return args1 + args2
        }
        function sub(args1, args2) {
            return args1 - args2
        }
        class Person {
            name: string
            age: number
            constructor(name, age){}
            introduce() {}
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


class TestWorkerSymbolicBreakpoints(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15998,
            'debugger_server_port': 15989,
            'bundle_name': 'com.example.multiworker10',
            'hap_name': 'MultiWorker10.hap',
            'hap_path': str(resource_path / 'hap' / 'MultiWorker10.hap'),
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
        # main thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='add')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", main_thread, params)
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
        # main thread: Debugger.paused,hit symbolic breakpoint,function name is add
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "add")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 78)
        ################################################################################################################
        # main thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='sub')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused,hit symbolic breakpoint,function name is sub
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "sub")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 82)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
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
        # worker thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='sub'),
                               debugger.SymbolicBreakpoint(functionName='add')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", worker_thread, params)
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
        ################################################################################################################
        # worker thread: Debugger.paused
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused,hit symbolic breakpoint,function name is add
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "add")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 37)
        ################################################################################################################
        # main thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='setAge')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", worker_thread, params)
        ################################################################################################################
        # main thread: Debugger.removeSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='add')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.removeSymbolicBreakpoints", worker_thread, params)
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused,hit symbolic breakpoint,function name is sub
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "sub")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 42)
        ################################################################################################################
        # worker thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", worker_thread)
        ################################################################################################################
        # worker thread: Debugger.paused,hit symbolic breakpoint,function name is setAge
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", worker_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['worker'])
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "setAge")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 59)
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

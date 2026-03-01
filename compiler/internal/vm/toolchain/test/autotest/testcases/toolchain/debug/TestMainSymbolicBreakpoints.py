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
#文 件 名：                 TestMainSymbolicBreakpoints.py
#文件说明：                 主线程 debug 调试 SetSymbolicBreakpoints、RemoveSymbolicBreakpoints
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程运行等待的调试器
    4.  主线程解析 entry_ability 和 index 文件
    5.  主线程 Index.ts 文件设置多个符号断点并运行
    6.  主线程 resume 验证是否正确命中断点
    7.  主线程验证命中符号断点位置
    8.  主线程删除符号断点
    9.  关闭主线程 debugger server 和 connect server 连接
#==================================================================
关键代码：
    Index.ets
        function add() : string {
            let num = "-binary";
            return num + min();
        }


        function min() : string {
            let num = "-binary";
            return num;
        }

        class Person {
            name: string = '';
            surname: string = '';
            constructor (n: string, sn: string) {
                this.name = n;
                this.surname = sn;
            }
            fullName(): string {
                //返回姓名
                return this.name + '' + this.surname;
          }
        }

        class Animal {
            name: string = '';
            constructor (n: string) {
                this.name = n;
            }
            className(): string {
                // 123
                return this.name;
            }
        }


        function main() : string {
            let num = "-binary";
            let person = new Person("12", "34")
            let lion = new Animal("lion")
            num = person.fullName() + add() + min() + lion.className()
            return num
        }
        main();
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


class TestMainSymbolicBreakpoints(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15789,
            'debugger_server_port': 15790,
            'bundle_name': 'com.example.maininstance05',
            'hap_name': 'MainInstance05.hap',
            'hap_path': str(resource_path / 'hap' / 'MainInstance05.hap'),
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
        # main thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='fullName'),
                               debugger.SymbolicBreakpoint(functionName='min'),
                               debugger.SymbolicBreakpoint(functionName='add'),
                               debugger.SymbolicBreakpoint(functionName='className')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused,hit symbolic breakpoint,function name is fullName
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "fullName")
        self.common_utils.assert_equal(response['params']['reason'], "Symbol")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 81)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused,hit symbolic breakpoint,function name is add
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "add")
        self.common_utils.assert_equal(response['params']['reason'], "Symbol")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 65)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused,hit symbolic breakpoint,function name is min
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "min")
        self.common_utils.assert_equal(response['params']['reason'], "Symbol")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 69)
        ################################################################################################################
        # main thread: Debugger.removeSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='min')]
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        await self.debugger_impl.send("Debugger.removeSymbolicBreakpoints", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused,hit symbolic breakpoint,function name is className
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "className")
        self.common_utils.assert_equal(response['params']['reason'], "Symbol")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 91)
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
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

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
#文 件 名：                 TestSwitchContextDebugger.py
#文件说明：                 测试切换Context后的debugger功能
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程运行等待调试器
    4.  主线程解析 entry_ability 和 index 文件
    5.  主线程点击屏幕按钮，加载插件
    6.  主线程解析 hms_ability，hms，plugins文件
    7.  主线程 plugins 文件设置断点并运行
    8.  主线程点击屏幕按钮，运行打车插件
    9.  主线程解析 calc，car，car_index文件
    10. 主线程car文件设置断点并运行
    11. 主线程resume验证是否正确命中断点
    12. 主线程 evaluateOnCallFrame，记录globalThis的hash值
    13. 主线程执行stepInto，验证是否正确命中car文件断点
    14. 主线程 evaluateOnCallFrame，验证变量值
    15. 主线程 evaluateOnCallFrame，记录切换Context后globalThis的hash值，是否与第一次不同
    16. 主线程执行stepOut，验证是否正确命中plugins文件断点，此时context再次切换
    17. 主线程 evaluateOnCallFrame，验证变量值
    18. 主线程 evaluateOnCallFrame，记录切换Context后globalThis的hash值，是否与第一次相同
    19. 主线程 removeBreakpointsByUrl，移除所有断点
    20. 主线程 setSymbolicBreakpoints，设置符号断点在callCar函数
    21. 主线程resume，恢复运行
    22. 主线程点击屏幕按钮，运行打车插件
    23. 主线程验证是否正确命中符号断点
    24. 主线程 smartStepInto，验证是否正确命中car断点
    25. 主线程 stepOut，验证是否正确命中car断点
    26. 主线程 stepOver，验证是否正确命中car断点
    27. 主线程 dropFrame，验证是否正确回退上一帧
    28. 主线程 removeSymbolicBreakpoints，移除符号断点
    29. 主线程 resume，恢复运行
    30. 关闭主线程debugger server和connect server连接
#==================================================================
关键代码：
    Car.ts
        class Car {
            add(a:number, b:number) : number {
                return a + b;
            }

            sub(a:number, b:number) : number {
                return a - b;
            }

            callCar(): string {
                let num = this.add(1, 2) + this.sub(3, 4);
                let str = 'yes. call car success. from ' + globalThis.aaa + 'num = ' + aa;
                return str;
            }
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


class TestSwitchContextDebugger(TestCase):
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
            'bundle_name': 'com.example.myapplication',
            'hap_name': 'ContextVerification01.hap',
            'hap_path': [str(resource_path / 'hap' / 'ContextVerification01' / 'ContextVerification01.hap'),
                         str(resource_path / 'hap' / 'ContextVerification01' / 'car.hsp')],
            'file_path': {
                'entry_ability': 'entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ts',
                'index': 'entry|entry|1.0.0|src/main/ets/pages/Index.ts',
                'hms_ability': 'entry|entry|1.0.0|src/main/ets/framework/hms_ability.ts',
                'hms': 'entry|entry|1.0.0|src/main/ets/framework/hms.ts',
                'plugins': 'entry|entry|1.0.0|src/main/ets/pages/plugins.ts',
                'calc': 'car|car|1.0.0|src/main/ets/utils/Calc.ts',
                'car': 'car|car|1.0.0|src/main/ets/plugin/Car.ts',
                'car_index': 'car|car|1.0.0|Index.ts',
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
        main_thread = await websocket.connect_to_debugger_server(self.config['pid'], True)
        ################################################################################################################
        # main thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", main_thread)
        ################################################################################################################
        # main thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", main_thread)
        ################################################################################################################
        # main thread: Debugger.setMixedDebugEnabled
        ################################################################################################################
        params = debugger.setMixedDebugEnabledParams(enabled=True, mixed_stack_enabled=False)
        await self.debugger_impl.send("Debugger.setMixedDebugEnabled", main_thread, params)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread)
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # EntryAbility
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # index
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: click on the screen（加载插件）
        ################################################################################################################
        self.ui_utils.click_on_middle()
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # hms_ability
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # hms
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # plugins
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['plugins'], line_number=70))]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:70:0' + self.config['file_path']['plugins'])
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # calc
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # car
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread) # car_index
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['car'], line_number=10))]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:10:0' + self.config['file_path']['car'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['plugins'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        # check globalThis hash: Object@hash1
        expression = (
            'UEFOREEAAAAAADAAAIIAAAAAAAAAAAAAMAAAAAAAA8BAAAAAASAAAAAAAAAAABMAAAAAAAAAAAAAogAAAANQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'awuDAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAs3md+//WERLPOOOOOOOOOOOOOOOOOUY7777777777777777ASDAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDD')
        params = debugger.EvaluateOnCallFrameParams(expression)
        response = await self.debugger_impl.send('Debugger.evaluateOnCallFrame', main_thread, params)
        object_hash1 = response['result']['result']['description']
        ################################################################################################################
        # main thread: Debugger.stepInto
        ################################################################################################################
        await self.debugger_impl.send('Debugger.stepInto', main_thread)
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['car'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        # check 'aa'
        expression = (
            'UEFOREEAAAAAADAAAIIAAAAAAAAAAAAAMAAAAAAAA8BAAAAAASAAAAAAAAAAABMAAAAAAAAAAAAAogAAAANQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'awuDAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAs3md+//WERLPOOOOOOOOOOOOOOOOOUY7777777777777777ASDAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDD')
        params = debugger.EvaluateOnCallFrameParams(expression)
        response = await self.debugger_impl.send('Debugger.evaluateOnCallFrame', main_thread, params)
        desc = response['result']['result']['description']
        self.common_utils.assert_equal(desc, "21")
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        # check 'aaa'
        expression = (
            'UEFOREEAAAAAADAAAIIAAAAAAAAAAAAAMAAAAAAAA8BAAAAAASAAAAAAAAA22222222222AAAAAAogAAAANQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7ufffffffffffffLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'awuDAAAIIAA/////////////AAdsdertydsAAAASAAAAs3md+//WERLPOOOOOOOOOOOOOOOOOUY7777777777777777ASDAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAXXXXXXXXXXXXXXXAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAqaaaasdfA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDD')
        params = debugger.EvaluateOnCallFrameParams(expression)
        response = await self.debugger_impl.send('Debugger.evaluateOnCallFrame', main_thread, params)
        desc = response['result']['result']['description']
        self.common_utils.assert_equal(desc, "i am from plugin")
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        # context switched. check globalThis hash: Object@hash2, hash2 should not equal to hash1
        expression = (
            'UEFOREEAAAAAADAAAIIAAAAAAAAAAAAAMAAAAAAAA8BAAAAAASAAAAAAAAA22222222222AAAAAAogAAAANQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7ufffffffffffffLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'awuDAAAIIAA/////////////AAdsdertydsAAAASAAAAs3md+//WERLPOOOOOOOOOOOOOOOOOUY7777777777777777ASDAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAXXXXXXXXXXXXXXXAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAqaaaasdfA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDD')
        params = debugger.EvaluateOnCallFrameParams(expression)
        response = await self.debugger_impl.send('Debugger.evaluateOnCallFrame', main_thread, params)
        object_hash2 = response['result']['result']['description']
        self.common_utils.assert_not_equal(object_hash1, object_hash2)
        ################################################################################################################
        # main thread: Debugger.stepOut
        ################################################################################################################
        await self.debugger_impl.send('Debugger.stepOut', main_thread)
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['plugins'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        expression = (
            'UEFOREEAAAAAADAAAIIAAAAAAAAAAAAAMAAAAAAAA8BAAAAAASAAAAAAAAA22222222222AAAAAAogAAAANQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7ufffffffffffffLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'awuDAAAIIAA/////////////AAdsdertydsAAAASAAAAs3md+//WERLPOOOOOOOOOOOOOOOOOUY7777777777777777ASDAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAXXXXXXXXXXXXXXXAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAqaaaasdfA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDD')
        params = debugger.EvaluateOnCallFrameParams(expression)
        response = await self.debugger_impl.send('Debugger.evaluateOnCallFrame', main_thread, params)
        desc = response['result']['result']['description']
        self.common_utils.assert_equal(desc, "i am from host")
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        # check 'aaa'
        expression = (
            'UEFOREEAAAAAADAAAIIAAAAAAAAAAAAAMAAAAAAAA8BAAAAAASAAAAAAAAA22222222222AAAAAAogAAAANQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAAAAAAAAAA7ufffffffffffffLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'awuDAAAIIAA/////////////AAdsdertydsAAAASAAAAs3md+//WERLPOOOOOOOOOOOOOOOOOUY7777777777777777ASDAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAXXXXXXXXXXXXXXXAAAAAAAAAA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDDDD'
            'UEFOREEAAAAAADAAAIIAA/////////////AAAAAAA8BAAAAAASAAqaaaasdfA7uYYYYAXCVBNMLLLLLLLLLQAAAAAAAAAAAAHAQDDDDD')
        params = debugger.EvaluateOnCallFrameParams(expression)
        response = await self.debugger_impl.send('Debugger.evaluateOnCallFrame', main_thread, params)
        object_hash3 = response['result']['result']['description']
        self.common_utils.assert_equal(object_hash3, object_hash1)
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['plugins'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['index'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['car'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolic_breakpoints = [debugger.SymbolicBreakpoints(functionName='callCar')]
        params = debugger.SymbolicBreakpoints(symbolic_breakpoints)
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: click on the screen
        ################################################################################################################
        self.ui_utils.click_on_middle()
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'Symbol')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['car'])
        self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], "callCar")
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 22)
        ################################################################################################################
        # main thread: Debugger.smartStepInto
        ################################################################################################################
        params = debugger.SmartStepIntoParams(url=self.config['file_path']['car'], line_number=16)
        response = await self.debugger_impl.send("Debugger.smartStepInto", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['car'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 16)
        ################################################################################################################
        # main thread: Debugger.stepOut
        ################################################################################################################
        await self.debugger_impl.send('Debugger.stepOut', main_thread)
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['car'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 22)
        ################################################################################################################
        # main thread: Debugger.stepOver
        ################################################################################################################
        await self.debugger_impl.send('Debugger.stepOver', main_thread)
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['car'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 23)
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        await self.debugger_impl.send('Debugger.dropFrame', main_thread)
        response = await self.debugger_impl.recv('Debugger.paused', main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['plugins'])
        self.common_utils.assert_equal(response['params']['reason'], 'other')
        self.common_utils.assert_equal(response['params']['callFrames'][0]['location']['lineNumber'], 70)
        ################################################################################################################
        # main thread: Debugger.removeSymbolicBreakpoints
        ################################################################################################################
        symbolic_breakpoints = [debugger.SymbolicBreakpoints(functionName='callCar')]
        params = debugger.SymbolicBreakpoints(symbolic_breakpoints)
        await self.debugger_impl.send("Debugger.removeSymbolicBreakpoints", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
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
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
#文 件 名：                 TestMainBasicFuncWithGPSOff.py
#文件说明：                 定位关闭状态下的调试性能测试
#!!================================================================
"""
import sys
import time
from datetime import datetime
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))  # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils, PerformanceUtils
from cdp import debugger, runtime
from implement_api import debugger_api, runtime_api


class TestMainBasicFuncWithGPSOff(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.performance_utils = PerformanceUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15800,
            'debugger_server_port': 15801,
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
            },
            'print_protocol': False  # 表示不会打印测试时用到的调试协议
        }

    def setup(self):
        Step('1.锁频锁核')
        self.performance_utils.lock_for_performance()
        Step('2.下载应用')
        self.driver.install_app(self.config['hap_path'], "-r")
        Step('3.启动设置应用')
        self.driver.start_app(package_name=self.config['bundle_name'], params=self.config['start_mode'])
        self.config['pid'] = self.common_utils.get_pid(self.config['bundle_name'])
        assert self.config['pid'] != 0, f'Failed to get pid of {self.config["bundle_name"]}'
        Step('4.设置屏幕常亮')
        self.ui_utils.keep_awake()
        Step('5.关闭位置信息')
        self.ui_utils.disable_location()
        Step('6.端口映射，连接server')
        self.common_utils.connect_server(self.config)
        self.debugger_impl = debugger_api.DebuggerImpl(self.id_generator, self.config['websocket'])
        self.runtime_impl = runtime_api.RuntimeImpl(self.id_generator, self.config['websocket'])
        Step('7.开启线程用于获取日志中的性能数据')
        self.hilog_process, self.perf_records_thread = self.performance_utils.get_perf_data_from_hilog()

    def process(self):
        Step('8.初始化websocket和taskpool')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        Step('9.执行测试用例')
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception
        Step('10.导出数据')
        time.sleep(3)
        self.hilog_process.stdout.close()
        self.hilog_process.terminate()
        self.hilog_process.wait()
        self.perf_records_thread.join()
        self.performance_utils.show_performance_data_in_html()

    def teardown(self):
        Step('11.关闭设置应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('12.卸载设置应用')
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
        await self.debugger_impl.send("Debugger.enable", main_thread)
        ################################################################################################################
        # main thread: Debugger.setSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='testDebug')]
        for i in range(1, 51):
            symbolicBreakpoints.append(debugger.SymbolicBreakpoint(functionName=f'testDebug{i}'))
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        start_time = datetime.now()
        await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", main_thread, params)
        end_time = datetime.now()
        self.performance_utils.add_time_data("SetSymbolicBreakpoints", 10, (end_time - start_time).microseconds // 1000)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
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
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['sendable'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['person'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['params'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['params'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['params'], line_number=84)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl",
                                                 main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:84:0:' + self.config['file_path']['params'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['drop_frame'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['drop_frame'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['drop_frame'], line_number=4)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl",
                                                 main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:4:0:' + self.config['file_path']['drop_frame'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['calculate'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['step'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['step'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        locations = [debugger.BreakLocationUrl(url=self.config['file_path']['step'], line_number=3)]
        params = debugger.SetBreakpointsLocations(locations)
        response = await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl",
                                                 main_thread, params)
        self.common_utils.assert_equal(response['result']['locations'][0]['id'],
                                       'id:3:0:' + self.config['file_path']['step'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        start_time = datetime.now()
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        end_time = datetime.now()
        self.performance_utils.add_time_data("ScriptParsed", 40, (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused, hit breakpoint
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:3:19:' + self.config['file_path']['step']])
        ################################################################################################################
        # main thread: Debugger.stepOut
        ################################################################################################################
        for i in range(500):
            # Debugger.stepInto
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.stepInto", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("StepInto", 8, (end_time - start_time).microseconds // 1000)
            # Debugger.stepOver
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.stepOver", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("StepOver", 8, (end_time - start_time).microseconds // 1000)
            # Debugger.stepInto
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.stepInto", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("StepInto", 8, (end_time - start_time).microseconds // 1000)
            # Debugger.stepOver
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.stepOver", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("StepOver", 8, (end_time - start_time).microseconds // 1000)
            # Debugger.smartStepInto
            params = debugger.SmartStepIntoParams(url=self.config['file_path']['calculate'], line_number=1)
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.smartStepInto", main_thread, params)
            await self.debugger_impl.send("Debugger.resume", main_thread)
            response = await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SmartStepInto", 15, (end_time - start_time).microseconds // 1000)
            self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], 'add')
            # Debugger.stepOut
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.stepOut", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("StepOut", 8, (end_time - start_time).microseconds // 1000)
            # Debugger.smartStepInto
            params = debugger.SmartStepIntoParams(url=self.config['file_path']['calculate'], line_number=4)
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.smartStepInto", main_thread, params)
            await self.debugger_impl.send("Debugger.resume", main_thread)
            response = await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SmartStepInto", 15, (end_time - start_time).microseconds // 1000)
            self.common_utils.assert_equal(response['params']['callFrames'][0]['functionName'], 'sub')
            # Debugger.stepOut
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.stepOut", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("StepOut", 8, (end_time - start_time).microseconds // 1000)
            # Debugger.resume
            await self.debugger_impl.send("Debugger.resume", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
        ################################################################################################################
        # main thread: Debugger.dropFrame
        ################################################################################################################
        params = debugger.DropFrameParams()
        for i in range(1000):
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.dropFrame", main_thread, params)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("DropFrame", 8, (end_time - start_time).microseconds // 1000)
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.resume", main_thread)
            await self.debugger_impl.recv("Debugger.paused", main_thread)
            end_time = datetime.now()
            self.performance_utils.add_time_data("ResumeAndPaused", 8, (end_time - start_time).microseconds // 1000)
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['drop_frame'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused, hit breakpoint
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:84:4:' + self.config['file_path']['params']])
        ################################################################################################################
        # main thread: Debugger.GetProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('0')
        for i in range(1000):
            start_time = datetime.now()
            response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetLocalProperties", 15, (end_time - start_time).microseconds // 1000)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], 'local')
        expected_variables = {'localArrayList': 'ArrayList', 'localBigInt64Array': 'BigInt64Array',
                              'localBigUint64Array': 'BigUint64Array', 'localDataView': 'DataView(20)',
                              'localDeque': 'Deque', 'localFloat32Array': 'Float32Array',
                              'localFloat64Array': 'Float64Array', 'localHashMap': 'HashMap',
                              'localHashSet': 'HashSet',
                              'localInt16Array': 'Int16Array(0)', 'localInt32Array': 'Int32Array(0)',
                              'localInt8Array': 'Int8Array(0)', 'localLightWeightMap': 'LightWeightMap',
                              'localLightWeightSet': 'LightWeightSet', 'localLinkedList': 'LinkedList',
                              'localList': 'List', 'localMapIter': 'function entries( { [native code] }',
                              'localNull': 'null', 'localPerson': 'Person', 'localPlainArray': 'PlainArray',
                              'localPromise': 'Promise', 'localProxy': 'Proxy', 'localQueue': 'Queue',
                              'localSendableClass': 'SendableClass[Sendable]',
                              'localSharedArrayBuffer': 'SharedArrayBuffer(32)', 'localStack': 'Stack',
                              'localTreeMap': 'TreeMap', 'localTreeSet': 'TreeSet',
                              'localUint16Array': 'Uint16Array',
                              'localUint32Array': 'Uint32Array', 'localUint8Array': 'Uint8Array(3)',
                              'localUint8ClampedArray': 'Uint8ClampedArray', 'localUndefined': 'undefined',
                              'localWeakMap': 'WeakMap(0)', 'localWeakRef': 'WeakRef {}',
                              'localWeakSet': 'WeakSet(0)'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # main thread: Debugger.GetProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('1')
        for i in range(1000):
            start_time = datetime.now()
            response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetClosureProperties", 6,
                                                 (end_time - start_time).microseconds // 1000)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], 'closure')
        expected_variables = {'closureArray': 'Array(3)', 'closureArrayBuffer': 'Arraybuffer(20)',
                              'closureMap': 'Map(0)', 'closureNum': '20', 'closureRegExp': '/^ab+c/g',
                              'closureSet': "Set(1) {'closure'}", 'closureString': 'closure'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # main thread: Debugger.GetProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('2')
        for i in range(1000):
            start_time = datetime.now()
            response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetModuleProperties", 15,
                                                 (end_time - start_time).microseconds // 1000)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], '')
        expected_variables = {'LocalParams': 'function LocalParams( { [js code] }',
                              'ArrayList': 'function ArrayList( { [native code] }',
                              'Deque': 'function Deque( { [native code] }',
                              'HashMap': 'function HashMap( { [native code] }',
                              'HashSet': 'function HashSet( { [native code] }',
                              'LightWeightMap': 'function LightWeightMap( { [native code] }',
                              'LightWeightSet': 'function LightWeightSet( { [native code] }',
                              'LinkedList': 'function LinkedList( { [native code] }',
                              'List': 'function List( { [native code] }',
                              'Person': 'function Person( { [js code] }',
                              'PersonHandler': 'function PersonHandler( { [js code] }',
                              'PlainArray': 'function PlainArray( { [native code] }',
                              'Queue': 'function Queue( { [native code] }',
                              'SendableClass': 'function SendableClass( { [js code] }[Sendable]',
                              'Stack': 'function Stack( { [native code] }',
                              'TreeMap': 'function TreeMap( { [native code] }',
                              'TreeSet': 'function TreeSet( { [native code] }'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # main thread: Debugger.GetProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('3')
        for i in range(1000):
            start_time = datetime.now()
            response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetGlobalProperties", 50,
                                                 (end_time - start_time).microseconds // 1000)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], 'global')
        expected_variables = {'globalArray': 'Array(3)', 'globalBigInt': '9007199254740991n',
                              'globalBool': 'Boolean{[[PrimitiveValue]]: false}',
                              'globalDate': 'Wed Aug 28 2024 02:41:00 GMT+0800',
                              'globalNum': 'Number{[[PrimitiveValue]]: 20}',
                              'globalObject': 'String{[[PrimitiveValue]]: globalObject}',
                              'globalStr': 'String{[[PrimitiveValue]]: globalStr}', 'globalThis': 'Object'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        SET_LOCALPERSON_AGE_22 = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAApAAAANcAAACAAAAAZAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pAAAANcAAACAAAAAhQAAAJcAAAAHYWdlACFkZWJ1Z2dlckdldFZhbHVlABdsb2NhbFBl'
            'cnNvbgAzTF9FU1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP'
            '//ygAAAIgCASEBAAACAAVWAQAABhQBAAAAFVNsb3ROdW1iZXIAAAABAAgBAAAGAAAANwkDLABEkEShRLJtYQVgBUIAAQBhBj4CAGEHAmEI'
            'YAYrAgcIYQRiFgAAAEMEAAAEZAtrAQ8A/////w8AAgAlAFEBAAA=')
        params = debugger.EvaluateOnCallFrameParams(SET_LOCALPERSON_AGE_22)
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SetLocalParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "22", "description": "22"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('closureString = "modified"')
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SetClosureParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "string", "unserializableValue": "modified", "description": "modified"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        SET_LIST_LENGTH_1 = (
            'UEFOREEAAAAAAAAADAACAGABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFwBAAAAAAAARAAAAAEAAABEAAAAoAAAANMAAACAAAAAYAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////oAAAANMAAACAAAAAhgAAAJgAAAAJTGlzdAAhZGVidWdnZXJHZXRWYWx1ZQANbGVuZ3Ro'
            'ADNMX0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAAEA///GAA'
            'AAiAIBHQEAAAIABVIBAAAGEAEAAAAVU2xvdE51bWJlcgAAAAEABAEAAAYAAAA3CQMsAESQRKFEsm1hBWAFQgABAGEGPgAAYQcCYQhgBisC'
            'BwhhBGIBAAAAQwQCAARkC2sBDwD/////DwACACUATQEAAA==')
        params = debugger.EvaluateOnCallFrameParams(SET_LIST_LENGTH_1)
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SetModuleParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result'],
                                       {"code": 1, "message": "TypeError: Cannot assign to read only property"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        SET_GLOBALARRAY_INDEX_2_999 = (
            'UEFOREEAAAAAAAAADAACAGQBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAGABAAAAAAAARAAAAAEAAABEAAAAmwAAAM4AAAB8AAAAZAEAAAIAAA'
            'BsAAAAAgAAAHQAAAD/////////////////////mwAAAM4AAAB8AAAAjgAAACFkZWJ1Z2dlckdldFZhbHVlABdnbG9iYWxBcnJheQAzTF9F'
            'U1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP//wQAAAIgCAR'
            'gBAAACAAVTAQAABgsBAAAAFVNsb3ROdW1iZXIAAAABAP8AAAAGAAAANwkDMgBEkEShRLJtYQVgBUIAAABhBj4BAGEHAmEIYAYrAgcIYQRi'
            'AgAAAGEFYucDAAA4BAQFZAtrAQ8A/////w8AAgArAAAAAE4BAAA=')
        params = debugger.EvaluateOnCallFrameParams(SET_GLOBALARRAY_INDEX_2_999)
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SetGlobalParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "999", "description": "999"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        LOCALPERSON_AGE = (
            'UEFOREEAAAAAAAAADAACAGABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFwBAAAAAAAARAAAAAEAAABEAAAApAAAANcAAACAAAAAYAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////pAAAANcAAACAAAAAhQAAAJcAAAAHYWdlACFkZWJ1Z2dlckdldFZhbHVlABdsb2NhbFBl'
            'cnNvbgAzTF9FU1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP'
            '//ygAAAIgCASEBAAACAAVSAQAABhQBAAAAFVNsb3ROdW1iZXIAAAABAAgBAAAGAAAANwkDKABEkEShRLJtYQVgBUIAAQBhBj4CAGEHAmEI'
            'YAYrAgcIYQRgBEIEAABkC2sBDwD/////DwACACEATQEAAA==')
        params = debugger.EvaluateOnCallFrameParams(LOCALPERSON_AGE)
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetLocalParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "22", "description": "22"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        params = debugger.EvaluateOnCallFrameParams('closureString')
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetClosureParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "string", "unserializableValue": "modified", "description": "modified"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        LIST_LENGTH = (
            'UEFOREEAAAAAAAAADAACAFwBAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFgBAAAAAAAARAAAAAEAAABEAAAAoAAAANMAAACAAAAAXAEAAAIAAA'
            'BsAAAAAwAAAHQAAAD/////////////////////oAAAANMAAACAAAAAhgAAAJgAAAAJTGlzdAAhZGVidWdnZXJHZXRWYWx1ZQANbGVuZ3Ro'
            'ADNMX0VTU2xvdE51bWJlckFubm90YXRpb247AAAAAACBQAAAAgAAF2Z1bmNfbWFpbl8wABNMX0dMT0JBTDsAAAAAAAEAAQIAAAEA///GAA'
            'AAiAIBHQEAAAIABU4BAAAGEAEAAAAVU2xvdE51bWJlcgAAAAEABAEAAAYAAAA3CQMoAESQRKFEsm1hBWAFQgABAGEGPgAAYQcCYQhgBisC'
            'BwhhBGAEQgQCAGQLawEPAP////8PAAIAIQBJAQAA')
        params = debugger.EvaluateOnCallFrameParams(LIST_LENGTH)
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetModuleParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "0", "description": "0"})
        ################################################################################################################
        # main thread: Debugger.evaluateOnCallFrame
        ################################################################################################################
        GLOBALARRAY_INDEX_2 = (
            'UEFOREEAAAAAAAAADAACAGABAAAAAAAAAAAAAAIAAAA8AAAAAQAAAFwBAAAAAAAARAAAAAEAAABEAAAAmwAAAM4AAAB8AAAAYAEAAAIAAA'
            'BsAAAAAgAAAHQAAAD/////////////////////mwAAAM4AAAB8AAAAjgAAACFkZWJ1Z2dlckdldFZhbHVlABdnbG9iYWxBcnJheQAzTF9F'
            'U1Nsb3ROdW1iZXJBbm5vdGF0aW9uOwAAAAAAgUAAAAIAABdmdW5jX21haW5fMAATTF9HTE9CQUw7AAAAAAABAAECAAABAP//wQAAAIgCAR'
            'gBAAACAAVPAQAABgsBAAAAFVNsb3ROdW1iZXIAAAABAP8AAAAGAAAANwkDLgBEkEShRLJtYQVgBUIAAABhBj4BAGEHAmEIYAYrAgcIYQRi'
            'AgAAAGEFYAU3BARkC2sBDwD/////DwACACcAAAAASgEAAA==')
        params = debugger.EvaluateOnCallFrameParams(GLOBALARRAY_INDEX_2)
        for i in range(1000):
            start_time = datetime.now()
            response = await self.debugger_impl.send("Debugger.evaluateOnCallFrame", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("GetGlobalParam", 6,
                                                 (end_time - start_time).microseconds // 1000)
        self.common_utils.assert_equal(response['result']['result'],
                                       {"type": "number", "unserializableValue": "999", "description": "999"})
        ################################################################################################################
        # main thread: Debugger.getPossibleAndSetBreakpointByUrl
        ################################################################################################################
        breakpoints_url = debugger.RemoveBreakpointsUrl(self.config['file_path']['params'])
        locations = []
        for i in range(1, 101):
            locations.append(debugger.BreakLocationUrl(url=self.config['file_path']['params'], line_number=i))
        breakpoints_locations = debugger.SetBreakpointsLocations(locations)
        for i in range(1000):
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, breakpoints_url)
            end_time = datetime.now()
            self.performance_utils.add_time_data("RemoveBreakpoints", 6,
                                                 (end_time - start_time).microseconds // 1000)
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.getPossibleAndSetBreakpointsByUrl", main_thread,
                                          breakpoints_locations)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SetManyBreakpoints", 50,
                                                 (end_time - start_time).microseconds // 1000)
        ################################################################################################################
        # main thread: Debugger.removeBreakpointsByUrl
        ################################################################################################################
        params = debugger.RemoveBreakpointsUrl(self.config['file_path']['params'])
        await self.debugger_impl.send("Debugger.removeBreakpointsByUrl", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.removeSymbolicBreakpoints
        ################################################################################################################
        symbolicBreakpoints = [debugger.SymbolicBreakpoint(functionName='testDebug')]
        for i in range(1, 51):
            symbolicBreakpoints.append(debugger.SymbolicBreakpoint(functionName=f'testDebug{i}'))
        params = debugger.SymbolicBreakpoints(symbolicBreakpoints)
        start_time = datetime.now()
        await self.debugger_impl.send("Debugger.removeSymbolicBreakpoints", main_thread, params)
        end_time = datetime.now()
        self.performance_utils.add_time_data("RemoveSymbolicBreakpoints", 10,
                                             (end_time - start_time).microseconds // 1000)
        for i in range(999):
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.setSymbolicBreakpoints", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("SetSymbolicBreakpoints", 10,
                                                    (end_time - start_time).microseconds // 1000)
            start_time = datetime.now()
            await self.debugger_impl.send("Debugger.removeSymbolicBreakpoints", main_thread, params)
            end_time = datetime.now()
            self.performance_utils.add_time_data("RemoveSymbolicBreakpoints", 10,
                                                    (end_time - start_time).microseconds // 1000)
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
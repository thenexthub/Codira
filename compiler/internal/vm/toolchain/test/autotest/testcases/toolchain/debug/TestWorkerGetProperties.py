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
#文 件 名：                 TestWorkerGetProperties.py
#文件说明：                 多实例 debug 调试 getProperties
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
    9.  子线程 getProperties 获取 local/closure/module/global作用域变量信息（Debugger.getProperties）
    10. 销毁子线程，对应的 debugger server 连接断开
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
from cdp import debugger, runtime
from implement_api import debugger_api, runtime_api


class TestWorkerGetProperties(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15644,
            'debugger_server_port': 15645,
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
        # worker thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('0')
        response = await self.runtime_impl.send("Runtime.getProperties", worker_thread, params)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], 'local')
        expected_variables = {'localArrayList': 'ArrayList', 'localBigInt64Array': 'BigInt64Array',
                              'localBigUint64Array': 'BigUint64Array', 'localDataView': 'DataView(20)',
                              'localDeque': 'Deque', 'localFloat32Array': 'Float32Array',
                              'localFloat64Array': 'Float64Array', 'localHashMap': 'HashMap', 'localHashSet': 'HashSet',
                              'localInt16Array': 'Int16Array(0)', 'localInt32Array': 'Int32Array(0)',
                              'localInt8Array': 'Int8Array(0)', 'localLightWeightMap': 'LightWeightMap',
                              'localLightWeightSet': 'LightWeightSet', 'localLinkedList': 'LinkedList',
                              'localList': 'List', 'localMapIter': 'function entries( { [native code] }',
                              'localNull': 'null', 'localPerson': 'Person', 'localPlainArray': 'PlainArray',
                              'localPromise': 'Promise', 'localProxy': 'Proxy', 'localQueue': 'Queue',
                              'localSendableClass': 'SendableClass[Sendable]',
                              'localSharedArrayBuffer': 'SharedArrayBuffer(32)', 'localStack': 'Stack',
                              'localTreeMap': 'TreeMap', 'localTreeSet': 'TreeSet', 'localUint16Array': 'Uint16Array',
                              'localUint32Array': 'Uint32Array', 'localUint8Array': 'Uint8Array(3)',
                              'localUint8ClampedArray': 'Uint8ClampedArray', 'localUndefined': 'undefined',
                              'localWeakMap': 'WeakMap(0)', 'localWeakRef': 'WeakRef {}', 'localWeakSet': 'WeakSet(0)'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # worker thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('1')
        response = await self.runtime_impl.send("Runtime.getProperties", worker_thread, params)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], 'closure')
        expected_variables = {'closureArray': 'Array(3)', 'closureArrayBuffer': 'Arraybuffer(20)',
                              'closureMap': 'Map(0)', 'closureNum': '20', 'closureRegExp': '/^ab+c/g',
                              'closureSet': "Set(1) {'closure'}", 'closureString': 'closure'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # worker thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('2')
        response = await self.runtime_impl.send("Runtime.getProperties", worker_thread, params)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], '')
        expected_variables = {'ArrayList': 'function ArrayList( { [native code] }',
                              'Deque': 'function Deque( { [native code] }',
                              'HashMap': 'function HashMap( { [native code] }',
                              'HashSet': 'function HashSet( { [native code] }',
                              'LightWeightMap': 'function LightWeightMap( { [native code] }',
                              'LightWeightSet': 'function LightWeightSet( { [native code] }',
                              'LinkedList': 'function LinkedList( { [native code] }',
                              'List': 'function List( { [native code] }',
                              'PlainArray': 'function PlainArray( { [native code] }',
                              'Queue': 'function Queue( { [native code] }',
                              'Stack': 'function Stack( { [native code] }',
                              'TreeMap': 'function TreeMap( { [native code] }',
                              'TreeSet': 'function TreeSet( { [native code] }', 'worker': 'Object'}
        self.common_utils.assert_equal(variables, expected_variables)
        ################################################################################################################
        # worker thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('3')
        response = await self.runtime_impl.send("Runtime.getProperties", worker_thread, params)
        variables = CommonUtils.get_variables_from_properties(response['result']['result'], 'global')
        expected_variables = {'globalArray': 'Array(3)', 'globalBigInt': '9007199254740991n',
                              'globalBool': 'Boolean{[[PrimitiveValue]]: false}',
                              'globalDate': 'Wed Aug 28 2024 02:41:00 GMT+0800',
                              'globalNum': 'Number{[[PrimitiveValue]]: 20}',
                              'globalObject': 'String{[[PrimitiveValue]]: globalObject}',
                              'globalStr': 'String{[[PrimitiveValue]]: globalStr}', 'globalThis': 'Object'}
        self.common_utils.assert_equal(variables, expected_variables)
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
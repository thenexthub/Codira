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
#文 件 名：                 TestPropertyWithRange.py
#文件说明：                 单实例 debug 调试，测试容器类函数分段请求能力
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime 和 Debugger
    3.  主线程 Index.ts 文件设置断点(Debugger.getPossibleAndSetBreakpointByUrl)
    4.  主线程 resume，暂停在下一段点(Debugger.resume)
    5.  主线程 getProperties，并指定需要请求的范围，返回给定对象的属性(Runtime.getProperties)
    6.  关闭主线程debugger server和connect server连接
#==================================================================
关键代码：
    Index.ets
        function testContainer() {
            // Array
            let array : Array<number> = new Array();
            for (let i = 0; i < 150; i++) {
                array[i] = i;
            }
            // Int8Array
            let int8Array : Int8Array = new Int8Array(150);
            for (let i = 0; i < 150; i++) {
                int8Array[i] = i;
            }
            // UInt8Array
            let uint8Array : UInt8Array = new UInt8Array(150);
            for (let i = 0; i < 150; i++) {
                uint8Array[i] = i;
            }
            // UInt8ClampedArray
            let uint8ClampedArray : UInt8ClampedArray = new UInt8ClampedArray(150);
            for (let i = 0; i < 150; i++) {
                uint8ClampedArray[i] = i;
            }
            // Int16Array
            let int16Array : Int16Array = new Int16Array(150);
            for (let i = 0; i < 150; i++) {
                int16Array[i] = i;
            }
            // UInt16Array
            let uint16Array : UInt16Array = new UInt16Array(150);
            for (let i = 0; i < 150; i++) {
                uint16Array[i] = i;
            }
            // Int32Array
            let int32Array : Int32Array = new Int32Array(150);
            for (let i = 0; i < 150; i++) {
                int32Array[i] = i;
            }
            // UInt32Array
            let uint32Array : UInt32Array = new UInt32Array(150);
            for (let i = 0; i < 150; i++) {
                uint32Array[i] = i;
            }
            // BigInt64Array
            let bigInt64Array : BigInt64Array = new BigInt64Array(150);
            for (let i = 0; i < 150; i++) {
                bigInt64Array[i] = 10000000n;
            }
            // BigUInt64Array
            let bigUInt64Array : BigUInt64Array = new BigUInt64Array(150);
            for (let i = 0; i < 150; i++) {
                bigUInt64Array[i] = 20000000n;
            }
            // Float32Array
            let float32Array : Float32Array = new Float32Array(150);
            for (let i = 0; i < 150; i++) {
                float32Array[i] = 1.1;
            }
            // Float64Array
            let float64Array : Float64Array = new Float64Array(150);
            for (let i = 0; i < 150; i++) {
                float64Array[i] = 1.1;
            }
            // ArrayList
            let arrayList : ArrayList<number> = new ArrayList();
            for (let i = 0; i < 150; i++) {
                arrayList.add(i);
            }
            // Deque
            let deque : Deque<number> = new Deque();
            for (let i = 0; i < 150; i++) {
                deque.insertEnd(i);
            }
            // HashMap
            let hashmap : HashMap<string, number> = new HashMap();
            for (let i = 0; i < 150; i++) {
                hashmap.set(i+"", i);
            }
            // HashSet
            let hashset : HashSet<number> = new HashSet();
            for (let i = 0; i < 150; i++) {
                hashset.add(i);
            }
            // LightWeightMap
            let lightWeightMap : LightWeightMap<string, number> = new LightWeightMap();
            for (let i = 0; i < 150; i++) {
                lightWeightMap.set(i+"", i);
            }
            // LightWeightSet
            let lightWeightSet : LightWeightSet<number> = new LightWeightSet();
            for (let i = 0; i < 150; i++) {
                lightWeightSet.add(i);
            }
            // LinkedList
            let linkedList : LinkedList<number> = new LinkedList();
            for (let i = 0; i < 150; i++) {
                linkedList.add(i);
            }
            // List
            let list : List<number> = new List();
            for (let i = 0; i < 150; i++) {
                list.add(i);
            }
            // PlainArray
            let plainArray : PlainArray<string> = new PlainArray();
            for (let i = 0; i < 150; i++) {
                list.add(i, i + "");
            }
            // Queue
            let queue : Queue<number> = new Queue;
            for (let i = 0; i < 150; i++) {
                queue.add(i);
            }
            // Stack
            let stack : Stack<number> = new Stack;
            for (let i = 0; i < 150; i++) {
                stack.push(i);
            }
            // TreeMap
            let treeMap : TreeMap<string, number> = new TreeMap();
            for (let i = 0; i < 150; i++) {
                treeMap.set(i+"", i);
            }
            // TreeSet
            let treeSet : TreeSet<number> = new TreeSet();
            for (let i = 0; i < 150; i++) {
                treeSet.add(i);
            }
            // Vector
            let vector : Vector<number> = new Vector();
            for (let i = 0; i < 150; i++) {
                vector.add(i);
            }
            // SendableMap
            let sendableMap : collections.Map<string, number> = new collections.Map();
            for (let i = 0; i < 150; i++) {
                sendableMap.set(i+"", i);
            }
            // Map
            let map : Map<string, number> = new Map();
            for (let i = 0; i < 150; i++) {
                map.set(i+"", i);
            }
            // WeakMap
            let weakMap : WeakMap<Object, string> = new WeakMap();
            for (let i = 0; i < 150; i++) {
                weakMap.set(new Object(i), i+"");
            }
            // SendableSet
            let sendableSet : collections.Set<number> = new collections.Set();
            for (let i = 0; i < 150; i++) {
                sendableSet.add(i);
            }
            // Set
            let set : Set<number> = new Set();
            for (let i = 0; i < 150; i++) {
                set.add(i);
            }
            // WeakSet
            let weakSet : WeakSet<Object> = new WeakSet();
            for (let i = 0; i < 150; i++) {
                weakSet.add(new Object(i));
            }
            // SendableArray
            let sendableArray : collections.Array<number> = new collections.Array();
            for (let i = 0; i < 150; i++) {
                sendableArray.push(i);
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


class TestConcurrentSymbolicBreakpoints(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15852,
            'debugger_server_port': 15853,
            'bundle_name': 'com.example.TestPropertyWithRange',
            'hap_name': 'PropertyWithRange.hap',
            'hap_path': str(resource_path / 'hap' / 'PropertyWithRange.hap'),
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
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
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
                                       'id:220:0' + self.config['file_path']['index'])
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.paused, hit breakpoint
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['hitBreakpoints'],
                                       ['id:220:4:' + self.config['file_path']['index']])
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('0', True, False, True)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'][0]['name'], 'testContainer')
        self.common_utils.assert_equal(response['result']['result'][0]['value']['type'], 'function')
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('11', True, False, True, 0, 10)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(len(response['result']['result']), 10)
        self.common_utils.assert_equal(response['result']['result'][0]['name'], '0')
        self.common_utils.assert_equal(response['result']['result'][0]['value']['type'], 'number')
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('15', True, False, True, 0, 50)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(len(response['result']['result']), 50)
        self.common_utils.assert_equal(response['result']['result'][0]['name'], '10')
        self.common_utils.assert_equal(response['result']['result'][0]['value']['type'], 'bigint')
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('19', True, False, True)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'][1]['name'], '[[Deque]]')
        self.common_utils.assert_equal(response['result']['result'][1]['value']['arrayOrContainer'], True)
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('43', True, False, True, 140, 20)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(len(response['result']['result']), 11)
        self.common_utils.assert_equal(response['result']['result'][0]['name'], '140')
        self.common_utils.assert_equal(response['result']['result'][0]['value']['type'], 'number')
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('25', True, False, True)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'][1]['name'], '[[HashMap]]')
        self.common_utils.assert_equal(response['result']['result'][1]['value']['arrayOrContainer'], True)
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('45', True, False, True, 120, 20)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(len(response['result']['result']), 20)
        self.common_utils.assert_equal(response['result']['result'][0]['name'], '120')
        self.common_utils.assert_equal(response['result']['result'][0]['value']['type'], 'object')
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('31', True, False, True)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(response['result']['result'][1]['name'], '[[Entries]]')
        self.common_utils.assert_equal(response['result']['result'][1]['value']['arrayOrContainer'], True)
        ################################################################################################################
        # main thread: Runtime.getProperties
        ################################################################################################################
        params = runtime.GetPropertiesParams('67', True, False, True, 0, 200)
        response = await self.runtime_impl.send("Runtime.getProperties", main_thread, params)
        self.common_utils.assert_equal(len(response['result']['result']), 151)
        self.common_utils.assert_equal(response['result']['result'][0]['name'], '0')
        self.common_utils.assert_equal(response['result']['result'][0]['value']['type'], 'number')
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
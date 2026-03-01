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
#文 件 名：                 TestAllocationMassiveMoveNode.py
#文件说明：                 Allocation 录制中大量创建string类型数据并打印
#!!================================================================
"""
import sys
import time
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))  # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils, PerformanceUtils
from cdp import heap_profiler
from implement_api import debugger_api, runtime_api, heap_profiler_api


class TestAllocationMassiveMoveNode(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.performance_utils = PerformanceUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '',
            'connect_server_port': 15810,
            'debugger_server_port': 15811,
            'bundle_name': 'com.example.performance03',
            'hap_name': 'Performance03.hap',
            'hap_path': str(resource_path / 'hap' / 'Performance03.hap'),
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
        self.heap_profiler_impl = heap_profiler_api.HeapProfilerImpl(self.id_generator, self.config['websocket'])
        Step('7.attach调试应用')
        self.common_utils.attach(self.config['bundle_name'])
        Step('8.开启线程用于获取日志中的性能数据')
        self.hilog_process, self.perf_records_thread = self.performance_utils.get_perf_data_from_hilog()


    def process(self):
        Step('9.初始化websocket和taskpool')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        Step('10.执行测试用例')
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception
        Step('11.导出数据')
        time.sleep(3)
        time_data = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        print("time_data: ", time_data.text)
        for t in time_data.text.split():
            self.performance_utils.add_time_data("AllocationMassiveMoveNode", 3500, int(t))
        self.hilog_process.stdout.close()
        self.hilog_process.terminate()
        self.hilog_process.wait()
        self.perf_records_thread.join()
        self.performance_utils.show_performance_data_in_html()

    def teardown(self):
        Step('12.关闭设置应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('13.卸载设置应用')
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
        # main thread: Runtime.getHeapUsage
        ################################################################################################################
        await self.runtime_impl.send("Runtime.getHeapUsage", main_thread)
        ################################################################################################################
        # main thread: HeapProfiler.startTrackingHeapObjects
        ################################################################################################################
        params = heap_profiler.TrackingHeapObjectsParams(False)
        await self.heap_profiler_impl.send("HeapProfiler.startTrackingHeapObjects", main_thread, params)
        ################################################################################################################
        # main thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", main_thread)
        ################################################################################################################
        # main thread: Execute Method
        ################################################################################################################
        for i in range(10):
            self.ui_utils.click_on_middle()
            time.sleep(2)
        ################################################################################################################
        # main thread: Runtime.getHeapUsage
        ################################################################################################################
        await self.runtime_impl.send("Runtime.getHeapUsage", main_thread)
        ################################################################################################################
        # main thread: HeapProfiler.stopTrackingHeapObjects
        ################################################################################################################
        await self.heap_profiler_impl.send("HeapProfiler.stopTrackingHeapObjects", main_thread)
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################

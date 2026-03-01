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
#文 件 名：                 TestSendableMethod.py
#文件说明：                 多次执行senable方法
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
from implement_api import debugger_api, runtime_api


class TestSendableMethod(TestCase):
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
            'connect_server_port': 15804,
            'debugger_server_port': 15805,
            'bundle_name': 'com.example.performance02',
            'hap_name': 'Performance02.hap',
            'hap_path': str(resource_path / 'hap' / 'Performance02.hap'),
            'file_path': {
                'entry_ability': 'entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ts',
                'index': 'entry|entry|1.0.0|src/main/ets/pages/Index.ts',
                'sendable': 'entry|entry|1.0.0|src/main/ets/resources/Sendable.ts',
            },
            'print_protocol': False
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
        Step('5.端口映射，连接server')
        self.common_utils.connect_server(self.config)
        self.debugger_impl = debugger_api.DebuggerImpl(self.id_generator, self.config['websocket'])
        self.runtime_impl = runtime_api.RuntimeImpl(self.id_generator, self.config['websocket'])
        Step('6.开启线程用于获取日志中的性能数据')
        self.hilog_process, self.perf_records_thread = self.performance_utils.get_perf_data_from_hilog()

    def process(self):
        Step('7.初始化websocket和taskpool')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        Step('8.执行测试用例')
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception
        Step('9.导出数据')
        time.sleep(3)
        self.hilog_process.stdout.close()
        self.hilog_process.terminate()
        self.hilog_process.wait()
        self.perf_records_thread.join()
        self.performance_utils.show_performance_data_in_html()

    def teardown(self):
        Step('10.关闭设置应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('11.卸载设置应用')
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
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resume", main_thread)
        ################################################################################################################
        # task thread: connect the debugger server
        ################################################################################################################
        task_thread = await self.debugger_impl.connect_to_debugger_server(self.config['pid'], False)
        ################################################################################################################
        # task thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", task_thread)
        ################################################################################################################
        # task thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", task_thread)
        ################################################################################################################
        # task thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", task_thread)
        ################################################################################################################
        # task thread: Set websocket for no more instance
        ################################################################################################################
        websocket.no_more_instance()
        ################################################################################################################
        # task thread: Test Sendable Method
        ################################################################################################################
        self.ui_utils.click_on_middle()
        await self.debugger_impl.recv("Debugger.scriptParsed", task_thread)
        response = await self.debugger_impl.recv("Debugger.paused", task_thread)
        await self.debugger_impl.send("Debugger.resume", task_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'], self.config['file_path']['sendable'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Test Sendable Method
        ################################################################################################################
        time.sleep(6)
        for i in range(4):
            self.ui_utils.click_on_middle()
            time.sleep(6)
        ################################################################################################################
        # main thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", main_thread)
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(task_thread.instance_id, task_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################
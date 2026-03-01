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
#文 件 名：                 TestWorkerRunAfterProfiler.py
#文件说明：                 测试调优后 worker 是否正常运行
#==================================================================
测试步骤：
    1.  连接 connect server 和主线程 debugger server
    2.  主线程使能 Runtime（Runtime.enable）
    3.  主线程设置采样间隔（Profiler.setSamplingInterval）
    4.  主线程启动调优（Profiler.start）
    5.  主线程去使能 Debugger（Debugger.disable）
    6.  等待 5 秒后关闭 CPU 调优，获取调优数据（Profiler.stop）
    7.  关闭主线程 debugger server 和 connect server 连接
    8.  触发点击事件，创建 worker 线程并与主线程通信，更新屏幕文字
#!!================================================================
关键代码：
    Index.ets
        .OnClick(() => {
            let myWorker = new worker.ThreadWorker("entry/ets/workers/Worker.ets")
            myWorker.onmessage = (e: MessageEvents) => {
                this.message = e.data
            }
        })
    Worker.ets
        const workerPort: ThreadWorkerGlobalScope = worker.workerPort;
        workerPort.onmessage = (e: MessageEvents) => {
            workerPort.postMessage(‘worker’)
        }
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
from all_utils import CommonUtils, UiUtils
from cdp import profiler
from implement_api import debugger_api, runtime_api, profiler_api


class TestWorkerRunAfterProfiler(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '',
            'connect_server_port': 15600,
            'debugger_server_port': 15601,
            'bundle_name': 'com.example.multiWorker06',
            'hap_name': 'MultiWorker06.hap',
            'hap_path': str(resource_path / 'hap' / 'MultiWorker06.hap')
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
        self.profiler_impl = profiler_api.ProfilerImpl(self.id_generator, self.config['websocket'])

    def process(self):
        Step('5.进行Profiler录制')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception
        Step('6.点击屏幕，判断屏幕文字是否更新')
        self.ui_utils.click_on_middle()
        time.sleep(1)
        contain = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(contain.text, 'worker')

    def teardown(self):
        Step('7.关闭应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('8.卸载应用')
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
        # main thread: Profiler.setSamplingInterval
        ################################################################################################################
        params = profiler.SamplingInterval(500)
        await self.profiler_impl.send("Profiler.setSamplingInterval", main_thread, params)
        ################################################################################################################
        # main thread: Profiler.start
        ################################################################################################################
        await self.profiler_impl.send("Profiler.start", main_thread)
        ################################################################################################################
        # main thread: Debugger.disable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.disable", main_thread)
        ################################################################################################################
        # main thread: sleep 5 seconds
        ################################################################################################################
        time.sleep(5)
        ################################################################################################################
        # main thread: Profiler.stop
        ################################################################################################################
        await self.profiler_impl.send("Profiler.stop", main_thread)
        ################################################################################################################
        # connect server: stopDebugger
        ################################################################################################################
        await websocket.send_msg_to_connect_server('stopDebugger')
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################
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
#文 件 名：                TestColdPatchHugeFile.py
#文件说明：                重载的文件超大，测试冷重载是否可以启动成功
#==================================================================
测试步骤：
    1、启动应用
    2、推入hqf
    3、触发热重载
    4、重启应用，触发冷重载
#==================================================================
关键代码：
    修改前：
        app-service.js
            new UTSJSONObject({
                id: "component.global-properties-events",
                name: "全局属性和时间-1002"
            })
    修改后：
        app-service.js
            new UTSJSONObject({
                id: "component.global-properties-events",
                name: "全局属性和时间-1003"
            })
#!!================================================================
"""
import sys
from pathlib import Path
from time import sleep

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))    # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step, CheckPoint
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils


class TestColdPatchHugeFile(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.pid = -1
        self.config = {
            'bundle_name': 'io.dcloud.uniappx',
            'hap_path': str(resource_path / 'hap' / 'UniAppDemo.hap'),

            'local_hqf_01_path': str(resource_path / 'hqf' / 'UniAppDemo.hqf'),
            'remote_hqf_01_path': '/data/UniAppDemo.hqf',
        }

    def setup(self):
        Step('1.安装应用')
        self.driver.install_app(self.config['hap_path'], "-r")
        Step('2.启动应用')
        self.driver.start_app(package_name=self.config['bundle_name'])
        self.pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert self.pid != 0, f'Failed to get pid of {self.config["bundle_name"]}'
        Step('3.设置屏幕常亮')
        self.ui_utils.keep_awake()

    def process(self):
        # Try close the permission request UI
        Step('4.尝试关闭权限请求窗口')
        request_component = self.driver.UiTree.find_component_by_path(
            '/WindowScene[0]/Stack/Row/UIExtensionComponent/root/Dialog/Column/Row')
        if request_component:
            button_permit = self.driver.UiTree.find_component_by_path(
                '/WindowScene[0]/Stack/Row/UIExtensionComponent/root/Dialog/'
                'Column/Row/Flex/Column/Scroll/Column/Row[3]/Flex/Button[1]')
            button_permit.click()
            request_component = self.driver.UiTree.find_component_by_path(
                '/WindowScene[0]/Stack/Row/UIExtensionComponent/root/Dialog/Column/Row')
            assert request_component is None, "permission request still exist"

        Step('5.触发热重载')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_01_path'],
                                      device_path=self.config['remote_hqf_01_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_01_path'])

        CheckPoint('确保应用正常运行')
        pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert pid == self.pid, f'App is no longer running with pid: {pid}'

        Step('6.退出应用')
        self.driver.stop_app(package_name=self.config['bundle_name'])

        CheckPoint('确保应用已退出')
        pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert pid == self.pid, f'App is still running with pid: {pid}'

        Step('7.重启应用，触发冷重载')
        self.driver.start_app(package_name=self.config['bundle_name'])

        CheckPoint('确保应用成功启动')
        # sleep to wait for the app to start successfully
        sleep(20)
        self.pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert pid == self.pid, f'Failed to start app {self.config["bundle_name"]}, cold patch failed'

    def teardown(self):
        Step("8.卸载应用")
        self.driver.uninstall_app(self.config['bundle_name'])
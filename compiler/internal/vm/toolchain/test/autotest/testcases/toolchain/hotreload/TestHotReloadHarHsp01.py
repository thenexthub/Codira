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
#文 件 名：                TestHotReloadHarHsp01.py
#文件说明：                在import Har和import Hsp的应用页面测试热重载
#==================================================================
测试步骤：
    1、启动应用
    2、点击屏幕中央，this.message被修改，并调用calc函数
    3、触发热重载
    4、再次点击屏幕中央，this.message被修改，并调用calc函数
#==================================================================
关键代码：
    修改前：
        import {Sum} from '@ohos/HarLibrary'
        import {Multiply} from '@ohos/HspDiscovery'
        .onClick(() => {
            this.message += '_'
            this.message += calc(1, 2)
        })
        function calc(x: number, y: number): number {
            console.log('TestHotReloadHarHsp01::calc(), args:', x, y)
            return Multiply(Sum(x, x), Sum(y, y))
        }
    修改后：
        import {Sum} from '@ohos/HarLibrary'
        import {Multiply} from '@ohos/HspDiscovery'
        .onClick(() => {
            this.message += '_'
            this.message += calc(3, 4)
        })
        function calc(x: number, y: number): number {
            console.log('TestHotReloadHarHsp01::calc(), args:', x, y)
            return Multiply(Sum(x, x), Sum(y, y))
        }
#!!================================================================
"""
import os
import sys
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))    # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step, CheckPoint
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils


class TestHotReloadHarHsp01(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.pid = -1
        self.config = {
            'bundle_name': 'com.example.HotReloadHarHsp01',
            'hap_path': [str(resource_path / 'hap' / 'HotReloadHarHsp01' / 'HotReloadHarHsp01.hap'),
                         str(resource_path / 'hap' / 'HotReloadHarHsp01' / 'HspDiscovery.hsp')],
            'local_hqf_entry_path': str(resource_path / 'hqf' / 'HotReloadHarHsp01.hqf'),
            'remote_hqf_entry_path': '/data/HotReloadHarHsp01.hqf'
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
        Step('4.获取主页Text控件')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        CheckPoint('检查主页的文本内容')
        self.common_utils.assert_equal(text_component.text, 'Hello World')

        Step('5.点击Text控件')
        text_component.click()
        CheckPoint('检查点击后主页的文本')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(text_component.text, 'Hello World_8')

        Step('6.触发热重载')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_entry_path'],
                                      device_path=self.config['remote_hqf_entry_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_entry_path'])

        Step('7.点击Text控件')
        text_component.click()
        CheckPoint('检查点击后主页的文本，是否符合热重载的修改')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(text_component.text, 'Hello World_8_48')

        CheckPoint('8.确保应用正常运行')
        pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert pid == self.pid, f'App is no longer running with pid: {pid}'

    def teardown(self):
        Step("9.卸载应用")
        self.driver.uninstall_app(self.config['bundle_name'])
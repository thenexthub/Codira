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
#文 件 名：                TestHotReloadClass01.py
#文件说明：                测试class中方法修改的热重载，覆盖DEFINECLASSWITHBUFFER_IMM8和DEFINECLASSWITHBUFFER_IMM16
#==================================================================
测试步骤：
    1、启动应用，加载Index页面，页面有Button0和Button1
    2、点击Button0
    3、进行第一次热重载（IMM8），再次点击Button0，按钮文本产生对应变化
    4、点击Button1
    5、进行第二次热重载（IMM16），再次点击Button1，按钮文本产生相应变化
#==================================================================
关键代码：
    修改前：
        Person.ets
            getName(): string {
                return this.name + ' ' + this.surname;
            }
        Point.ets
            getName() : string {
                return this.name
            }
    第一次修改：
        Person.ets
        getName(): string {
            return this.name + '-' + this.surname;
        }
    第二次修改：
        Point.ets
        getName() : string {
            return this.name + '-' +this.x + '-' +this.y
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


class TestHotReloadClass01(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.pid = -1
        self.config = {
            'bundle_name': 'com.example.HotReloadClass01',
            'hap_path': str(resource_path / 'hap' / 'HotReloadClass01.hap'),

            'local_hqf_01_path': str(resource_path / 'hqf' / 'HotReloadClass01.01.hqf'),
            'remote_hqf_01_path': '/data/HotReloadClass01.01.hqf',

            'local_hqf_02_path': str(resource_path / 'hqf' / 'HotReloadClass01.02.hqf'),
            'remote_hqf_02_path': '/data/HotReloadClass01.01.hqf',
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
        Step('4.获取主页Button0和Button1的控件')
        button_0 = self.driver.UiTree.find_component_by_path('/root/Row/Column/Button[0]/Text')
        CheckPoint('检查button_0的文本内容')
        self.common_utils.assert_equal(button_0.text, 'class IMM8')
        button_1 = self.driver.UiTree.find_component_by_path('/root/Row/Column/Button[1]/Text')
        CheckPoint('检查button_1的文本内容')
        self.common_utils.assert_equal(button_1.text, 'class IMM16')

        Step('5.点击Button0控件')
        button_0.click()
        CheckPoint('检查点击Button0后的文本')
        button_0 = self.driver.UiTree.find_component_by_path('/root/Row/Column/Button[0]/Text')
        self.common_utils.assert_equal(button_0.text, 'class IMM8 John Smith')

        Step('6.第一次触发热重载: DEFINECLASSWITHBUFFER_IMM8')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_01_path'],
                                      device_path=self.config['remote_hqf_01_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_01_path'])

        Step('7.点击Button0控件')
        button_0.click()
        CheckPoint('检查点击后Button0的文本，是否符合热重载的修改')
        button_0 = self.driver.UiTree.find_component_by_path('/root/Row/Column/Button[0]/Text')
        self.common_utils.assert_equal(button_0.text, 'class IMM8 John Smith John-Smith')

        Step('8.点击Button1控件')
        button_1.click()
        CheckPoint('检查点击button_1后的文本')
        button_1 = self.driver.UiTree.find_component_by_path('/root/Row/Column/Button[1]/Text')
        self.common_utils.assert_equal(button_1.text, 'class IMM16 Singularity_255')

        Step('9.第二次触发热重载: DEFINECLASSWITHBUFFER_IMM16')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_02_path'],
                                      device_path=self.config['remote_hqf_02_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_02_path'])

        Step('10.点击Button1控件')
        button_1.click()
        CheckPoint('检查点击后Button1的文本，是否符合热重载的修改')
        button_1 = self.driver.UiTree.find_component_by_path('/root/Row/Column/Button[1]/Text')
        self.common_utils.assert_equal(button_1.text, 'class IMM16 Singularity_255 Singularity_255-255-255')

        CheckPoint('11.确保应用正常运行')
        pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert pid == self.pid, f'App is no longer running with pid: {pid}'

    def teardown(self):
        Step("12.卸载应用")
        self.driver.uninstall_app(self.config['bundle_name'])
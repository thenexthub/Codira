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
#文 件 名：                TestHotReloadMultiModule.py
#文件说明：                同时修改hqf和hsp两个module中的文件，触发热重载后，再触发冷补丁
#==================================================================
测试步骤：
    1、启动应用，加载Index页面
    2、同时修改hqf和hsp中的文件（hqf引用了hsp），触发热重载
    3、退出应用后重启，触发冷补丁
#==================================================================
关键代码：
    修改前：
        hqf => Index.ets:
            import {separator} from './Styles'
            import {add} from "hsplibrary"
            .onClick(()=>{
                this.message += separator
                this.message += add(1, 2)
            })
        hsp => Calc.ets:
        import {factor} from './Factors'
        export function add(a: number, b: number) {
            return (a + b) * factor(a, b);
        }
    修改后：
        hqf => Index.ets:
            import {separator} from './Styles'
            import {add} from "hsplibrary"
            .onClick(()=>{
                this.message += separator
                this.message += add(2, 2)
            })
        hsp => Calc.ets:
        import {factor} from './Factors'
        export function add(a: number, b: number) {
            return (a + b) + factor(a, b);
        }
#!!================================================================
"""
import os
import sys
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))  # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step, CheckPoint
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils


class TestHotReloadMultiModule(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.pid = -1
        self.config = {
            'bundle_name': 'com.example.HotReloadHsp01',
            'hap_path': [str(resource_path / 'hap' / 'HotReloadHsp01' / 'HotReloadHsp01.hap'),
                         str(resource_path / 'hap' / 'HotReloadHsp01' / 'HspLibrary.hsp')],
            'local_hqf_path': [str(resource_path / 'hqf' / 'HotReloadHsp01' / 'HotReloadHsp01.hqf'),
                               str(resource_path / 'hqf' / 'HotReloadHsp01' / 'HspLibrary.hqf')],
            'remote_hqf_path': ['/data/HotReloadHsp01.hqf', '/data/HspLibrary.hqf']
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
        Step('3.获取主页Text控件')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        CheckPoint('检查主页的文本内容')
        self.common_utils.assert_equal(text_component.text, 'Hello World')

        Step('4.点击主页Text控件')
        text_component.click()
        CheckPoint('检查点击后的文本')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(text_component.text, 'Hello World_3')

        Step('5.同时修改hap和hsp，触发热重载')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_path'][0],
                                      device_path=self.config['remote_hqf_path'][0])
        self.driver.Storage.push_file(local_path=self.config['local_hqf_path'][1],
                                      device_path=self.config['remote_hqf_path'][1])
        self.common_utils.hot_reload(self.config['remote_hqf_path'])

        Step('6.点击主页Text控件')
        text_component.click()
        CheckPoint('检查点击后的文本')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(text_component.text, 'Hello World_3_6')

        Step('7.退出应用')
        self.driver.stop_app(package_name=self.config['bundle_name'])

        Step('8.重新启动应用，触发冷补丁')
        self.driver.start_app(package_name=self.config['bundle_name'])

        foreground_apps = self.driver.AppManager.get_foreground_apps()
        assert any(app['bundleName'] == self.config['bundle_name'] for app in foreground_apps), \
            'Cold patch: app start failed!'

        Step('9.点击主页Text控件')
        text_component.click()
        CheckPoint('检查点击后的文本')
        text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(text_component.text, 'Hello World_6')

    def teardown(self):
        Step("10.卸载应用")
        self.driver.uninstall_app(self.config['bundle_name'])
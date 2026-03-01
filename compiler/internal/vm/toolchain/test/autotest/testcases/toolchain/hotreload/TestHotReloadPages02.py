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
#文 件 名：                TestHotReloadPages02.py
#文件说明：                 跳转页面重载后，重新进入跳转页面，再次重载
#==================================================================
测试步骤：
    1、启动应用，加载Index页面
    2、点击进入Mine页面，点击屏幕
    3、修改Mine.ets，触发热重载，点击屏幕
    4、返回Index页面，重新进入Mine页面，点击屏幕
    4、再次修改Mine.ets，触发热重载，点击屏幕
#==================================================================
关键代码：
    修改前：
        Index.ets
            .onClick(() => {
              router.pushUrl({'url': 'pages/Mine'})
            })
        Mine.ets
        import {add} from './Utils'
        .onClick(() => {
          this.message += '_'
          this.message += add(1, 2)
        })
    第一次修改：
        Mine.ets
        .onClick(() => {
          this.message += '_'
          this.message += add(12, 34)
        })
    第二次修改：
        Mine.ets
        .onClick(() => {
          this.message += '_'
          this.message += add(56, 78)
        })
#!!================================================================
"""
import os
import sys
from pathlib import Path
from time import sleep

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))    # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step, CheckPoint
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils


class TestHotReloadPages02(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.pid = -1
        self.config = {
            'bundle_name': 'com.example.HotReloadPages01',
            'hap_path': str(resource_path / 'hap' / 'HotReloadPages01.hap'),

            'local_hqf_01_path': str(resource_path / 'hqf' / 'HotReloadPages01.03.hqf'),
            'remote_hqf_01_path': '/data/HotReloadPages01.03.hqf',

            'local_hqf_02_path': str(resource_path / 'hqf' / 'HotReloadPages01.04.hqf'),
            'remote_hqf_02_path': '/data/HotReloadPages01.04.hqf',
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
        Step('4.获取Index.ets页面Text控件')
        index_text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        CheckPoint('检查主页的文本和背景颜色')
        self.common_utils.assert_equal(index_text_component.text, 'Click Me')
        self.common_utils.assert_equal(index_text_component.attr['backgroundColor'], '#FFFF0000')

        Step('5.点击Index.ets，跳转到Mine.ets')
        index_text_component.click()
        CheckPoint('检查点击后Mine.ets的文本')
        mine_text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(mine_text_component.text, 'Mine View')

        Step('6.点击Mine.ets，触发文本变化')
        index_text_component.click()
        CheckPoint('检查点击后Mine.ets的文本')
        mine_text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(mine_text_component.text, 'Mine View_3')

        Step('7.修改Mine.ets，第一次触发热重载')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_01_path'],
                                      device_path=self.config['remote_hqf_01_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_01_path'])
        CheckPoint('检查热重载后，Mine.ets的文本')
        mine_text_component.click()
        mine_text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(mine_text_component.text, 'Mine View_3_46')

        Step('8.返回Index.ets页面')
        self.driver.go_back()
        sleep(2)

        Step('9.点击Index.ets，再次跳转到Mine.ets')
        index_text_component.click()
        sleep(2)
        mine_text_component.click()
        mine_text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(mine_text_component.text, 'Mine View_46')

        Step('10.修改Mine.ets，第二次触发热重载')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_02_path'],
                                      device_path=self.config['remote_hqf_02_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_02_path'])
        CheckPoint('检查热重载后，Mine.ets的文本')
        mine_text_component.click()
        mine_text_component = self.driver.UiTree.find_component_by_path('/root/RelativeContainer/Text')
        self.common_utils.assert_equal(mine_text_component.text, 'Mine View_46_134')

        CheckPoint('11.确保应用正常运行')
        pid = self.common_utils.get_pid(self.config['bundle_name'])
        assert pid == self.pid, self.driver.log_error(f'App is no longer running with pid: {pid}')

    def teardown(self):
        Step("12.卸载应用")
        self.driver.uninstall_app(self.config['bundle_name'])
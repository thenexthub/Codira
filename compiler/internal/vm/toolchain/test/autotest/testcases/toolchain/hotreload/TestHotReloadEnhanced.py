#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 Huawei Device Co., Ltd.
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
#文 件 名：                TestHotReloadEnhanced.py
#文件说明：                测试热重载对于动态加载、import *、懒加载、NAPI加载模块等场景的支持
#==================================================================
测试步骤：
    1、启动应用，加载Index页面
    2、依次点击页面中的所有按钮，判断按钮中的文本是否符合预期
    3、修改hap中的文件，触发热重载
    4、依次点击页面中的所有按钮，判断按钮中的文本是否符合预期
    5、退出应用后重启，触发冷补丁
    6、依次点击页面中的所有按钮，判断按钮中的文本是否符合预期
#==================================================================
关键代码：
    修改前：
        DynModuleB.ets:
            export function dynB() {
                return 2;
            }
        DynModuleB.ets:
            export function dynD() {
                return 2;
            }
        ExportAllB.ets:
            export function exportB() {
                return 2;
            }
        LazyModuleA.ets:
            export function lazyA() {
                return 8;
            }
        NapiTestB.ets:
            function napiTestB() {
                return 2;
            }
    修改后：
        DynModuleB.ets:
            export function dynB() {
                return 0;
            }
        DynModuleB.ets:
            export function dynD() {
                return 0;
            }
        ExportAllB.ets:
            export function exportB() {
                return 0;
            }
        LazyModuleA.ets:
            export function lazyA() {
                return 0;
            }
        NapiTestB.ets:
            function napiTestB() {
                return 0;
            }

#!!================================================================
"""
import sys
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))    # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step, CheckPoint
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils


class TestHotReloadEnhanced(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.pid = -1
        self.config = {
            'bundle_name': 'com.example.hotReloadEnhanced',
            'hap_path': str(resource_path / 'hap' / 'HotReloadEnhanced.hap'),
            'local_hqf_path': str(resource_path / 'hqf' / 'HotReloadEnhanced.hqf'),
            'remote_hqf_path': '/data/HotReloadEnhanced.hqf',
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
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[0]')
        text_component.click()
        CheckPoint('检查动态加载01')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[0]')
        self.common_utils.assert_equal(text_component.text, '动态加载0182')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[1]')
        text_component.click()
        CheckPoint('检查动态加载02')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[1]')
        self.common_utils.assert_equal(text_component.text, '动态加载0282')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[2]')
        text_component.click()
        CheckPoint('检查import *')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[2]')
        self.common_utils.assert_equal(text_component.text, 'import *82')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[3]')
        text_component.click()
        CheckPoint('检查lazy import')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[3]')
        self.common_utils.assert_equal(text_component.text, 'lazy import82')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[4]')
        text_component.click()
        CheckPoint('检查napiLdModuleWithInfo')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[4]')
        self.common_utils.assert_equal(text_component.text, 'napiLdModuleWithInfo10')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[5]')
        text_component.click()
        CheckPoint('检查napiLdModule')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[5]')
        self.common_utils.assert_equal(text_component.text, 'napiLdModule10')

        Step('5.修改hap，触发热重载')
        self.driver.Storage.push_file(local_path=self.config['local_hqf_path'],
                                      device_path=self.config['remote_hqf_path'])
        self.common_utils.hot_reload(self.config['remote_hqf_path'])

        Step('6.点击主页Text控件')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[0]')
        text_component.click()
        CheckPoint('检查动态加载01')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[0]')
        self.common_utils.assert_equal(text_component.text, '动态加载018280')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[1]')
        text_component.click()
        CheckPoint('检查动态加载02')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[1]')
        self.common_utils.assert_equal(text_component.text, '动态加载028280')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[2]')
        text_component.click()
        CheckPoint('检查import *')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[2]')
        self.common_utils.assert_equal(text_component.text, 'import *8280')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[3]')
        text_component.click()
        CheckPoint('检查lazy import')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[3]')
        self.common_utils.assert_equal(text_component.text, 'lazy import8202')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[4]')
        text_component.click()
        CheckPoint('检查napiLdModuleWithInfo')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[4]')
        self.common_utils.assert_equal(text_component.text, 'napiLdModuleWithInfo108')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[5]')
        text_component.click()
        CheckPoint('检查napiLdModule')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[5]')
        self.common_utils.assert_equal(text_component.text, 'napiLdModule108')

        Step('7.退出应用')
        self.driver.stop_app(package_name=self.config['bundle_name'])

        Step('8.重新启动应用，触发冷补丁')
        self.driver.start_app(package_name=self.config['bundle_name'])

        foreground_apps = self.driver.AppManager.get_foreground_apps()
        assert any(app['bundleName'] == self.config['bundle_name'] for app in foreground_apps), \
            'Cold patch: app start failed!'

        Step('9.点击主页Text控件')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[0]')
        text_component.click()
        CheckPoint('检查动态加载01')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[0]')
        self.common_utils.assert_equal(text_component.text, '动态加载0180')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[1]')
        text_component.click()
        CheckPoint('检查动态加载02')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[1]')
        self.common_utils.assert_equal(text_component.text, '动态加载0280')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[2]')
        text_component.click()
        CheckPoint('检查import *')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[2]')
        self.common_utils.assert_equal(text_component.text, 'import *80')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[3]')
        text_component.click()
        CheckPoint('检查lazy import')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[3]')
        self.common_utils.assert_equal(text_component.text, 'lazy import02')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[4]')
        text_component.click()
        CheckPoint('检查napiLdModuleWithInfo')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[4]')
        self.common_utils.assert_equal(text_component.text, 'napiLdModuleWithInfo8')

        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[5]')
        text_component.click()
        CheckPoint('检查napiLdModule')
        text_component = self.driver.UiTree.find_component_by_path('/root/Row/Scroll/Column/Button[5]')
        self.common_utils.assert_equal(text_component.text, 'napiLdModule8')

    def teardown(self):
        Step("10.卸载应用")
        self.driver.uninstall_app(self.config['bundle_name'])

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Preload before run test cases.
"""

from devicetest.core.test_case import TestCase, Step
from xdevice import platform_logger

from hypium import UiExplorer, BY

log = platform_logger("Preload")


# 类名必须是Preload
class Preload(TestCase):
    def __init__(self, configs):
        self.tag = self.__class__.__name__
        TestCase.__init__(self, self.tag, configs)

    def setup(self):
        Step("预置")

    def process(self):
        log.info("message {}".format(self.configs.get("testargs").get("pass_through", {})))
        # 任务可能使用多个设备,需逐一进行初始化
        for device in self.devices:
            d = UiExplorer(device)
            Step("********处理打卡软件和日志********")
            out_message1 = d.hdc("shell ls /sys_prod/app/MSPES")
            out_message2 = d.hdc("shell param get persist.sys.hilog.binary.on")
            flag = 0
            if "MSPES" in out_message1:
                log.info("********打卡软件处理********")
                d.Storage.remount("/sys_prod")
                d.hdc("shell rm /sys_prod/app/MSPES/MSPES.hap")
                flag = 1
            if "true" in out_message2:
                log.info("********日志处理********")
                d.hdc("shell param set persist.sys.hilog.binary.on false")
                flag = 1
            if flag == 1:
                log.info("********设备重启********")
                d.System.reboot()
                d.System.wait_for_boot_complete()
                d.shell("power-shell setmode 602")
                d.Screen.wake_up()
                d.ScreenLock.unlock()
                d.wait(5)
                has_dialog_usb0 = d.find_component(BY.type("Dialog"))
                if has_dialog_usb0:
                    d.touch(BY.text("取消"))
                d.wait(5)
            else:
                log.info("********打卡软件和日志都已处理********")

            Step("********清理锁屏密码********")
            d.ScreenLock.clear_password("123456", EXCEPTION=False)
            d.wait(5)

            out_message3 = d.hdc("shell aa dump -l")
            if "hwstartupguide" in out_message3:
                Step("********跳过开机向导********")
                d.AppManager.disable_startup_guide()
                d.wait(5)
            else:
                log.info("********已跳过开机向导********")

            Step("********处理代码签名后亮屏解锁处理小艺输入法********")
            d.shell("mount -o rw,remount /")
            d.hdc("target mount")
            d.wait(1)
            d.shell('sed -i "s%/proc/sys/kernel/xpm/xpm_mode 4%/proc/sys/kernel/xpm/xpm_mode 0%g" '
                    '/system/etc/init/key_enable.cfg')
            d.shell('sed -i "s%/proc/sys/kernel/xpm/xpm_mode 4%/proc/sys/kernel/xpm/xpm_mode 0%g" '
                    '/system/etc/init/key_enable.enable_xxpm.cfg')
            d.wait(1)
            d.System.reboot()
            d.System.wait_for_boot_complete()
            d.shell("power-shell setmode 602")
            d.Screen.wake_up()
            d.ScreenLock.unlock()
            d.wait(5)
            has_dialog_usb1 = d.find_component(BY.type("Dialog"))
            if has_dialog_usb1:
                d.touch(BY.text("取消"))
            d.shell("sysctl -w kernel.xpm.xpm_mode=0")
            # 使用时需手动修改下方bundle_name
            bundle_name = ""
            d.start_app(bundle_name, "MainAbility")
            if d.find_component(BY.text("同意")) is not None:
                d.touch(BY.text("同意"))
            if d.find_component(BY.text("用于允许不同设备间的数据交换")) is not None:
                d.touch(BY.text("允许"))
            if d.find_component(BY.id("createNote")) is not None:
                d.touch(BY.id("createNote"))
            if d.find_component(BY.text("小艺输入法")) is not None:
                d.touch(BY.text("同意"))
            if d.find_component(BY.text("下一步")) is not None:
                d.touch(BY.text("下一步"))
            d.AppManager.clear_recent_app()
            d.wait(5)
            
            Step("********关闭wifi********")
            result = d.Wifi.disable()
            log.info("********关闭wifi********" + str(result))

    def tear_down(self):
        Step("收尾")
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

Description: Action words of hdc fport.
"""


class Fport(object):
    def __init__(self, driver):
        self.driver = driver
        self.retry_times = 3
        self.increase_step = 7

    def fport_connect_server(self, port, pid, bundle_name):
        for _ in range(self.retry_times):
            fport_info = f"tcp:{port} ark:{pid}@{bundle_name}"
            if self.fport_server(fport_info):
                return port, fport_info
            else:  # The port may be occupied
                port += self.increase_step
        return -1, ''

    def fport_debugger_server(self, port, pid, tid=0):
        for _ in range(self.retry_times):
            if tid == 0:
                fport_info = f"tcp:{port} ark:{pid}@Debugger"
            else:
                fport_info = f"tcp:{port} ark:{pid}@{tid}@Debugger"
            if self.fport_server(fport_info):
                return port, fport_info
            else:  # The port may be occupied
                port += self.increase_step
        return -1, ''

    def fport_server(self, fport_info):
        cmd = f"fport {fport_info}"
        self.driver.log_info('fport_server: ' + cmd)
        result = self.driver.hdc(cmd)
        self.driver.log_info(result)
        return result == 'Forwardport result:OK'

    def rm_fport_server(self, fport_info):
        cmd = f"fport rm {fport_info}"
        self.driver.log_info('rm_port: ' + cmd)
        result = self.driver.hdc(cmd)
        assert 'success' in result, result

    def clear_fport(self):
        list_fport_cmd = 'fport ls'
        list_fport_result = self.driver.hdc(list_fport_cmd)
        self.driver.log_info(list_fport_result)
        if 'Empty' in list_fport_result:
            return
        for fport_item in [item for item in list_fport_result.split('[Forward]') if 'ark' in item]:
            un_fport_command = (f"fport rm {fport_item.split('    ')[1].split(' ')[0]} "
                                f"{fport_item.split('    ')[1].split(' ')[1]}")
            un_fport_result = self.driver.hdc(un_fport_command)
            self.driver.log_info(un_fport_command)
            self.driver.log_info(un_fport_result)
            assert 'success' in un_fport_result, un_fport_result

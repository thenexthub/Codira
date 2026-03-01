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

Description: Utils for action words.
"""

import asyncio
import os
import re
import subprocess
import threading
from typing import Union

from hypium import SystemUI, BY, MatchPattern

from fport import Fport
from taskpool import TaskPool
from toolchain_websocket import ToolchainWebSocket


class TimeRecord:
    def __init__(self, expected_time, actual_time):
        self.expected_time = expected_time
        self.actual_times = [actual_time]

    def avg_actual_time(self):
        return sum(self.actual_times) / len(self.actual_times)

    def max_actual_time(self):
        return max(self.actual_times)

    def min_actual_time(self):
        return min(self.actual_times)

    def add_actual_time(self, actual_time):
        self.actual_times.append(actual_time)

    def avg_proportion(self):
        return self.avg_actual_time() * 100 / self.expected_time

    def rounds(self):
        return len(self.actual_times)

    def mid_actual_time(self):
        self.actual_times.sort()
        return self.actual_times[len(self.actual_times) // 2]


class CommonUtils(object):
    def __init__(self, driver):
        self.driver = driver

    @staticmethod
    async def communicate_with_debugger_server(websocket, connection, command, message_id, counts=1):
        """
        counts:  represents the number of recv messages corresponding to one send message.
        Assembles and send the commands, then return the response.
        Send message to the debugger server corresponding to the to_send_queue.
        Return the response from the received_queue.
        """
        command['id'] = message_id
        await websocket.send_msg_to_debugger_server(connection.instance_id, connection.send_msg_queue, command)
        if counts > 1 :
            response = await websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                                   connection.received_msg_queue, counts)
        else:
            response = await websocket.recv_msg_of_debugger_server(connection.instance_id,
                                                               connection.received_msg_queue)
        return response

    @staticmethod
    def get_custom_protocols():
        protocols = ["removeBreakpointsByUrl",
                     "setMixedDebugEnabled",
                     "replyNativeCalling",
                     "getPossibleAndSetBreakpointByUrl",
                     "dropFrame",
                     "setNativeRange",
                     "resetSingleStepper",
                     "callFunctionOn",
                     "smartStepInto",
                     "saveAllPossibleBreakpoints"]
        return protocols

    @staticmethod
    def message_id_generator():
        message_id = 1
        while True:
            yield message_id
            message_id += 1

    @staticmethod
    def assert_equal(actual, expected):
        '''
        判断actual和expected是否相同,若不相同则抛出异常
        '''
        assert actual == expected, f"\nExpected: {expected}\nActual: {actual}"

    @staticmethod
    def assert_not_equal(actual, expected):
        '''
        判断actual和expected是否不同,若相同则抛出异常
        '''
        assert actual != expected, f"\nThe expected is same as the actual: {actual}"

    @staticmethod
    def get_variables_from_properties(properties, prefix_name):
        '''
        properties是Runtime.getProperties协议返回的变量信息
        该方法会根据prefix_name前缀匹配properties中的变量名,符合的变量会以字典形式返回变量名和相关描述
        描述首先采用description中的内容,但会删掉其中的哈希值,没有description则会采用subtype或type中的内容
        '''
        variables = {}
        for var in properties:
            if not var['name'].startswith(prefix_name):
                continue
            name = var['name']
            value = var['value']
            description = value.get('description')
            if description is not None:
                index_of_at = description.find('@')
                if index_of_at == -1:
                    variables[name] = description
                    continue
                variables[name] = description[:index_of_at]
                index_of_bracket = description.find('[', index_of_at + 1)
                if index_of_bracket != -1:
                    variables[name] += description[index_of_bracket:]
            else:
                subtype = value.get('subtype')
                variables[name] = subtype if subtype is not None else value.get('type')
        return variables

    def get_pid(self, bundle_name):
        '''
        获取bundle_name对应的pid
        '''
        pid = self.driver.shell("pidof " + bundle_name).strip()
        if not pid.isdigit():
            return 0
        self.driver.log_info(f'pid of {bundle_name}: ' + pid)
        return int(pid)

    def attach(self, bundle_name):
        '''
        通过bundle_name使指定应用进入调试模式
        '''
        attach_result = self.driver.shell(f"aa attach -b {bundle_name}").strip()
        self.driver.log_info(attach_result)
        self.assert_equal(attach_result, 'attach app debug successfully.')

    def connect_server(self, config):
        '''
        根据config连接ConnectServer
        '''
        config['websocket'] = ToolchainWebSocket(self.driver, config, config.get('print_protocol', True))
        config['taskpool'] = TaskPool()

    def hot_reload(self, hqf_path: Union[str, list]):
        '''
        根据hqf_path路径对应用进行热重载
        '''
        assert isinstance(hqf_path, (str, list)), 'Type of hqf_path is NOT string or list!'
        if isinstance(hqf_path, str):
            cmd = f'bm quickfix -a -f {hqf_path} -d'
        elif isinstance(hqf_path, list):
            cmd = f'bm quickfix -a -f {" ".join(hqf_path)} -d'
        self.driver.log_info('hot reload: ' + cmd)
        result = self.driver.shell(cmd).strip()
        self.driver.log_info(result)
        self.assert_equal(result, 'apply quickfix succeed.')

    async def async_wait_timeout(self, task, timeout=3):
        '''
        在timeout内执行task异步任务,若执行超时则抛出异常
        '''
        try:
            result = await asyncio.wait_for(task, timeout)
            return result
        except asyncio.TimeoutError:
            self.driver.log_error('await timeout!')

    def hdc_target_mount(self):
        '''
        挂载设备文件系统
        '''
        cmd = 'target mount'
        self.driver.log_info('Mount finish: ' + cmd)
        result = self.driver.hdc(cmd).strip()
        self.driver.log_info(result)
        self.assert_equal(result, 'Mount finish')

    def hdc_file_send(self, source, sink):
        '''
        将source中的文件发送到设备的sink路径下
        '''
        cmd = f'file send {source} {sink}'
        self.driver.log_info(cmd)
        result = self.driver.hdc(cmd)
        self.driver.log_info(result)
        assert 'FileTransfer finish' in result, result

    def clear_fault_log(self):
        '''
        清除故障日志
        '''
        cmd = 'rm /data/log/faultlog/faultlogger/*'
        self.driver.log_info(cmd)
        result = self.driver.shell(cmd)
        self.driver.log_info(result)

    def save_fault_log(self, log_path):
        '''
        保存故障日志到log_path
        '''
        if not os.path.exists(log_path):
            os.makedirs(log_path)

        cmd = f'file recv /data/log/faultlog/faultlogger/ {log_path}'
        self.driver.log_info(cmd)
        result = self.driver.hdc(cmd)
        self.driver.log_info(result)

    def remove_file(self, file):
        '''
        删除指定文件
        '''
        cmd = rf'rm {file}'
        self.driver.shell(cmd)

    def check_snapshot(self, path, pid):
        '''
        校验目标路径下是否生成了'jsheap-pid'格式的snapshot，且文件大小不为0
        '''
        cmd = rf'ls -alth {path} | grep jsheap-{pid}'
        result = self.driver.shell(cmd)
        parts = result.split()
        self.assert_equal(len(parts), 8)
        size_str = parts[4]
        self.assert_not_equal(size_str.strip(), '0')

    def try_confirm_usb_connection_method(self):
        '''
        设备重启后，会出现选择并确认USB连接方式的页面，该函数的作用为点击确定使该页面消失
        '''
        component = self.driver.UiTree.find_component_by_path(
            '/WindowScene[0]/Stack/Row/UIExtensionComponent/root/Dialog/Column/__Common__/JsView/Scroll/Column/JsView/'
            'Column[1]/Scroll/Column/List/ListItem[0]/Column/Button/Row/Flex/Text[0]')
        if component:
            self.assert_equal(component.text, '传输文件')
            confirm = self.driver.UiTree.find_component_by_path(
                '/WindowScene[0]/Stack/Row/UIExtensionComponent/root/Dialog/Column/__Common__/JsView/Scroll/Column/'
                'JsView/Column[2]/Row')
            confirm.click()
            component = self.driver.UiTree.find_component_by_path(
                '/WindowScene[0]/Stack/Row/UIExtensionComponent/root/Dialog/Column/__Common__/JsView/Scroll/Column/'
                'JsView/Column[1]/Scroll/Column/List/ListItem[0]/Column/Button/Row/Flex/Text[0]')
        self.assert_equal(component, None)

class UiUtils(object):
    def __init__(self, driver):
        self.driver = driver

    def get_screen_size(self):
        '''
        获取屏幕大小
        '''
        screen_info = self.driver.shell(f"hidumper -s RenderService -a screen")
        match = re.search(r'physical screen resolution: (\d+)x(\d+)', screen_info)
        # 新版本镜像中screen_info信息格式有变,需要用下方正则表达式获取
        if match is None:
            match = re.search(r'physical resolution=(\d+)x(\d+)', screen_info)
        assert match is not None, f"screen_info is incorrect: {screen_info}"
        return int(match.group(1)), int(match.group(2))

    def click(self, x, y):
        '''
        点击屏幕指定坐标位置
        '''
        click_result = self.driver.shell(f"uinput -T -c {x} {y}")
        self.driver.log_info(click_result)
        assert "click coordinate" in click_result, f"click_result is incorrect: {click_result}"

    def click_on_middle(self):
        '''
        点击屏幕中心
        '''
        width, height = self.get_screen_size()
        middle_x = width // 2
        middle_y = height // 2
        self.click(middle_x, middle_y)

    def back(self):
        """
        返回上一步
        """
        cmd = 'uitest uiInput keyEvent Back'
        self.driver.log_info('click the back button: ' + cmd)
        result = self.driver.shell(cmd).strip()
        self.driver.log_info(result)
        CommonUtils.assert_equal(result, 'No Error')

    def keep_awake(self):
        """
        保持屏幕常亮,要在屏幕亮起时使用该方法才有效
        """
        cmd = 'power-shell wakeup'
        result = self.driver.shell(cmd).strip()
        assert "WakeupDevice is called" in result, result
        cmd = 'power-shell timeout -o 2147483647'
        result = self.driver.shell(cmd).strip()
        assert "Override screen off time to 2147483647" in result, result
        cmd = 'power-shell setmode 602'
        result = self.driver.shell(cmd).strip()
        assert "Set Mode Success!" in result, result
        self.driver.log_info("Keep the screen awake Success!")

    def open_control_center(self):
        '''
        滑动屏幕顶部右侧打开控制中心
        '''
        width, height = self.get_screen_size()
        start = (int(width * 0.75), 20)
        end = (int(width * 0.75), int(height * 0.3))
        cmd = f"uinput -T -m {start[0]} {start[1]} {end[0]} {end[1]} 500"
        self.driver.log_info('open control center')
        result = self.driver.shell(cmd)
        self.driver.log_info(result)

    def click_location_component(self):
        '''
        点击控制中心中的位置控件
        '''
        self.open_control_center()
        self.driver.wait(1)
        width, height = self.get_screen_size()
        self.click(int(width * 0.2), int(height * 0.95))
        self.back()

    def disable_location(self):
        '''
        关闭位置信息（GPS）,不能在异步任务中使用
        '''
        self.driver.wait(1)
        SystemUI.open_control_center(self.driver)
        comp = self.driver.find_component(BY.key("ToggleBaseComponent_Image_location", MatchPattern.CONTAINS))
        result = comp.getAllProperties()
        try:
            bg = result.backgroundColor
            value = int("0x" + bg[3:], base=16)
            # 读取背景颜色判断开启和关闭
            if value < 0xffffff:
                comp.click()
                self.driver.wait(1)
            self.driver.press_back()
        except Exception as e:
            raise RuntimeError("Fail to disable location")

    def enable_location(self):
        '''
        开启位置信息（GPS）,不能在异步任务中使用
        '''
        self.driver.wait(1)
        SystemUI.open_control_center(self.driver)
        comp = self.driver.find_component(BY.key("ToggleBaseComponent_Image_location", MatchPattern.CONTAINS))
        result = comp.getAllProperties()
        try:
            bg = result.backgroundColor
            value = int("0x" + bg[3:], base=16)
            # 读取背景颜色判断开启和关闭
            if value == 0xffffff:
                comp.click()
                self.driver.wait(1)
            self.driver.press_back()
        except Exception as e:
            raise RuntimeError("Fail to enable location")


class PerformanceUtils(object):
    def __init__(self, driver):
        self.driver = driver
        self.time_data = {}
        self.file_path_hwpower_genie_engine = "/system/app"
        self.file_name_hwpower_genie_engine = "HwPowerGenieEngine3"
        self.cpu_num_list = [0] * 3
        self.cpu_start_id = [0] * 3
        self.total_cpu_num = 0
        self.cpu_cluster_num = 0
        self.prev_mode = 0
        self.board_ipa_support = False
        self.perfg_version = "0.0"

    def show_performance_data_in_html(self):
        '''
        将性能数据以html格式展示出来
        '''
        header = ['测试用例', '执行次数', '期望执行时间(ms)', '最大执行时间(ms)',
                  '最小执行时间(ms)', '执行时间中位数(ms)', '平均执行时间(ms)',
                  '平均执行时间/期望执行时间(%)']
        content = []
        failed_cases = []
        for key, value in self.time_data.items():
            if value.avg_proportion() >= 130:
                failed_cases.append(key)
            content.append([key, value.rounds(), value.expected_time, value.max_actual_time(),
                            value.min_actual_time(), value.mid_actual_time(),
                            f'{value.avg_actual_time():.2f}',
                            f'{value.avg_proportion():.2f}'])
        table_html = '<table border="1" cellpadding="5" cellspacing="0">'
        header_html = '<thead>'
        for i in header:
            header_html += f'<th>{i}</th>'
        header_html += '</thead>'
        content_html = '<tbody>'
        for row in content:
            row_html = '<tr style="color: red;">' if row[0] in failed_cases else '<tr>'
            for i in row:
                row_html += f'<td>{i}</td>'
            row_html += '</tr>'
            content_html += row_html
        table_html += header_html
        table_html += content_html
        table_html += '</table>'
        self.driver.log_info(table_html)
        assert len(failed_cases) == 0, "The following cases have obvious deterioration: " + " ".join(failed_cases)

    def add_time_data(self, case_name: str, expected_time: int, actual_time: int):
        '''
        添加指定性能用例对应的耗时
        '''
        if case_name in self.time_data:
            self.time_data[case_name].add_actual_time(actual_time)
        else:
            self.time_data[case_name] = TimeRecord(expected_time, actual_time)

    def get_perf_data_from_hilog(self):
        '''
        从hilog日志中获取性能数据
        '''
        # config the hilog
        cmd = 'hilog -r'
        self.driver.log_info(cmd)
        result = self.driver.shell(cmd)
        assert 'successfully' in result, result

        cmd = 'hilog -G 16M'
        self.driver.log_info(cmd)
        result = self.driver.shell(cmd)
        assert 'successfully' in result, result

        cmd = 'hilog -Q pidoff'
        self.driver.log_info(cmd)
        result = self.driver.shell(cmd)
        assert 'successfully' in result, result

        cmd = 'hilog -Q domainoff'
        self.driver.log_info(cmd)
        result = self.driver.shell(cmd)
        assert 'successfully' in result, result

        cmd = 'hilog -b INFO'
        self.driver.log_info(cmd)
        result = self.driver.shell(cmd)
        assert 'successfully' in result, result

        # create a sub-process to receive the hilog
        hilog_process = subprocess.Popen(['hdc', 'shell', 'hilog'],
                                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        def get_time_from_records(records):
            # 解析record的数据
            pattern = r"\((.*?)\) Expected Time = (\d+), Actual Time = (\d+)"
            for record in records:
                match = re.search(pattern, record)
                if match:
                    expected_time = int(match.group(2))
                    actual_time = int(match.group(3))
                    case_name = match.group(1)
                    if case_name in self.time_data:
                        self.time_data[case_name].add_actual_time(actual_time)
                    else:
                        self.time_data[case_name] = TimeRecord(expected_time, actual_time)

        def get_perf_records():
            records = []
            try:
                for line in iter(hilog_process.stdout.readline, b''):
                    decode_line = line.decode('utf-8')
                    if '[ArkCompilerPerfTest]' in decode_line:
                        records.append(decode_line)
            except ValueError:
                self.driver.log_info('hilog stream is closed.')
            finally:
                get_time_from_records(records)

        perf_records_thread = threading.Thread(target=get_perf_records)
        perf_records_thread.start()

        return hilog_process, perf_records_thread

    def lock_for_performance(self):
        '''
        执行性能用例前先进行锁频锁核操作
        '''
        if self._check_if_already_locked():
            self.driver.log_info("The device has locked the frequency and core.")
            return True
        # 获取系统参数
        self._check_if_support_ipa()
        self._check_perf_genius_version()
        if not self._get_core_number():
            self.driver.log_info("Get core number failed.")
            return False
        # 取消性能限制
        self._disable_perf_limitation()
        # 锁频
        self._lock_frequency()
        self._lock_ddr_frequency()
        # 锁中核和大核
        self._lock_core_by_id(self.cpu_start_id[1], self.total_cpu_num - 1)
        return True

    def _check_if_support_ipa(self):
        '''
        壳温IPA支持情况检查,检查结果存在 self.board_ipa_support 中
        '''
        result = self.driver.shell("cat /sys/class/thermal/thermal_zone1/type").strip()
        self.board_ipa_support = (result == 'board_thermal')
        self.driver.log_info('If support IPA: ' + str(self.board_ipa_support))

    def _check_perf_genius_version(self):
        '''
        perfGenius版本号检查
        '''
        result = self.driver.shell("ps -ef | grep perfg")
        idx = result.find('perfgenius@')
        if idx != -1:
            start_idx = idx + len('perfgenius') + 1
            self.perfg_version = result[start_idx:start_idx + 4]
        self.driver.log_info('PerfGenius Version: ' + self.perfg_version)

    def _get_core_number(self):
        '''
        从设备上读取大中小核个数
        '''
        # 获取CPU总个数
        result = self.driver.shell("cat /sys/devices/system/cpu/possible").strip()
        idx = result.find('-')
        if idx == -1 or result[idx + 1:] == '0':
            self.driver.log_info('Get total cpu num failed')
            return False
        self.total_cpu_num = int(result[idx + 1:]) + 1
        self.driver.log_info('total_cpu_num = ' + str(self.total_cpu_num))
        # 获取CPU小核个数
        result = self.driver.shell("cat /sys/devices/system/cpu/cpu0/topology/core_siblings_list").strip()
        idx = result.find('-')
        if idx == -1 or result[idx + 1:] == '0':
            self.driver.log_info('Get small-core cpu num failed')
            return False
        self.cpu_start_id[1] = int(result[idx + 1:]) + 1
        self.driver.log_info('self.cpu_start_id[1] = ' + str(self.cpu_start_id[1]))
        # 获取CPU中核个数
        result = self.driver.shell(
            f"cat /sys/devices/system/cpu/cpu{self.cpu_start_id[1]}/topology/core_siblings_list").strip()
        idx = result.find('-')
        if idx == -1 or result[idx + 1:] == '0':
            self.driver.log_info('Get medium-core cpu num failed')
            return False
        self.cpu_start_id[2] = int(result[idx + 1:]) + 1
        if self.cpu_start_id[2] == self.total_cpu_num:
            self.cpu_start_id[2] = self.cpu_start_id[1]
            return True
        self.driver.log_info('self.cpu_start_id[2] = ' + str(self.cpu_start_id[2]))
        # 获取CPU大核个数
        result = self.driver.shell(
            f"cat /sys/devices/system/cpu/cpu{self.cpu_start_id[2]}/topology/core_siblings_list").strip()
        if result == '':
            self.driver.log_info('Get large-core cpu num failed')
            return False
        idx = result.find('-')
        tmp_total_cpu_num = (int(result) if idx == -1 else int(result[idx + 1:])) + 1
        if tmp_total_cpu_num != self.total_cpu_num:
            self.driver.log_info('Get large-core cpu num failed')
            return False
        self.cpu_num_list[0] = self.cpu_start_id[1]
        self.cpu_num_list[1] = self.cpu_start_id[2] - self.cpu_start_id[1]
        self.cpu_num_list[2] = self.total_cpu_num - self.cpu_start_id[2]
        self.driver.log_info(f'Small-core cpu number: {self.cpu_num_list[0]};'
                             f'Medium-core cpu number: {self.cpu_num_list[1]};'
                             f'Large-core cpu number: {self.cpu_num_list[2]};')
        return True

    def _disable_perf_limitation(self):
        '''
        查杀省电精灵并关闭IPA和PerfGenius
        '''
        # 如果存在省电精灵,强制删除并重启手机
        result = self.driver.shell("find /system/app -name HwPowerGenieEngine3").strip()
        if result != '':
            self.driver.shell("rm -rf /system/app/HwPowerGenieEngine3")
            self.driver.System.reboot()
            self.driver.System.wait_for_boot_complete()
        # 关闭IPA
        self.driver.shell("echo disabled > /sys/class/thermal/thermal_zone0/mode")
        if self.board_ipa_support:
            self.driver.shell("echo disabled > /sys/class/thermal/thermal_zone1/mode")
            self.driver.shell("echo 10000 > /sys/class/thermal/thermal_zone1/sustainable_power")
        # 关闭perfGenius
        if self.perfg_version == '0.0':
            self.driver.shell("dumpsys perfhub --perfhub_dis")
        else:
            self.driver.shell(f"lshal debug vendor.huawei.hardware.perfgenius@{self.perfg_version}"
                              f"::IPerfGenius/perfgenius --perfgenius_dis")

    def _lock_frequency_by_start_id(self, start_id):
        '''
        锁id为start_id的CPU核的频率
        '''
        self.driver.log_info(f"Lock frequency, start_id = {start_id}")
        result = self.driver.shell(
            f"cat /sys/devices/system/cpu/cpu{start_id}/cpufreq/scaling_available_frequencies").strip()
        freq_list = list(map(int, result.split()))
        max_freq = max(freq_list)
        self.driver.shell(f"echo {max_freq} >  /sys/devices/system/cpu/cpu{start_id}/cpufreq/scaling_max_freq")
        self.driver.shell(f"echo {max_freq} >  /sys/devices/system/cpu/cpu{start_id}/cpufreq/scaling_min_freq")
        self.driver.shell(f"echo {max_freq} >  /sys/devices/system/cpu/cpu{start_id}/cpufreq/scaling_max_freq")

    def _lock_ddr_frequency(self):
        '''
        DDR锁频,锁最接近749000000的频率
        '''
        std_freq = 749000000
        self.driver.log_info("Lock ddr frequency")
        result = self.driver.shell("cat /sys/class/devfreq/ddrfreq/available_frequencies").strip()
        freq_list = list(map(int, result.split()))
        freq = min(freq_list, key=lambda x: abs(x - std_freq))
        self.driver.shell(f"echo {freq} >  /sys/class/devfreq/ddrfreq/max_freq")
        self.driver.shell(f"echo {freq} >  /sys/class/devfreq/ddrfreq/min_freq")
        self.driver.shell(f"echo {freq} >  /sys/class/devfreq/ddrfreq/max_freq")
        self.driver.shell(f"echo {freq} >  /sys/class/devfreq/ddrfreq_up_threshold/max_freq")
        self.driver.shell(f"echo {freq} >  /sys/class/devfreq/ddrfreq_up_threshold/min_freq")
        self.driver.shell(f"echo {freq} >  /sys/class/devfreq/ddrfreq_up_threshold/max_freq")

    def _lock_frequency(self):
        '''
        锁CPU的所有核的频率
        '''
        # 小核锁频
        self._lock_frequency_by_start_id(self.cpu_start_id[0])
        # 中核锁频
        if self.cpu_num_list[1] != 0:
            self._lock_frequency_by_start_id(self.cpu_start_id[1])
        # 大核锁频
        self._lock_frequency_by_start_id(self.cpu_start_id[2])

    def _lock_core_by_id(self, start_id, end_id):
        '''
        锁start_id到end_id的cpu核
        '''
        for i in range(start_id, end_id + 1):
            self.driver.shell(f"echo 0 > sys/devices/system/cpu/cpu{i}/online")
        self.driver.shell("echo test > sys/power/wake_unlock")

    def _check_if_already_locked(self):
        '''
        根据DDR频率来判断是否已经锁频锁核
        '''
        result = self.driver.shell("cat /sys/class/devfreq/ddrfreq/cur_freq").strip()
        return result == "749000000"
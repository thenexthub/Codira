#!/usr/bin/env python
# coding=utf-8

#
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import subprocess
import unittest
import os

from js_test_base import JsTestBase


class JsTestCase(JsTestBase):
    repeat_time = 3

    @classmethod
    def setUpClass(cls):
        os.environ["isSkipped"] = "False"
        # pass
        cls.get_hdctool(cls)
        hap_name = os.environ.get("HapName", "")
        dbg_name = "com.example"
        # start hap
        subprocess.run(
            [
                cls.hdctool,
                "shell",
                "aa",
                "start",
                "-a",
                "EntryAbility",
                "-b",
                hap_name,
            ],
            stdout=subprocess.PIPE,
        )
        cls.run_arkdb_server(cls, dbg_name)
        cls.connect_client_socket(cls, dbg_name)
        cls.tcp_client_socket.send("enable".encode('utf-8'))
        data, addr = cls.tcp_client_socket.recvfrom(cls.BUFSIZ)
        data, addr = cls.tcp_client_socket.recvfrom(cls.BUFSIZ)
        print("recv: ", data.decode('utf-8'))

    @classmethod
    def tearDownClass(cls):
        cls.close_client_socket(cls)
        # stop hap
        hap_name = os.environ.get("HapName", "")
        subprocess.run(
            [
                cls.hdctool,
                "shell",
                "aa",
                "force-stop",
                hap_name,
            ],
            stdout=subprocess.PIPE,
        )

    def test_set_and_delete_breakpoints(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        for i in breakpoint_lines:
            self.send_command("delete 1")

    def test_repeat_breakpoints(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            for i in range(self.repeat_time):
                self.send_command("b entry/src/main/ets/pages/%s" % line)
        for i in breakpoint_lines:
            self.send_command("delete 1")

    def test_dispaly(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
            self.send_command("display")
        for i in range(len(breakpoint_lines)):
            self.send_command("delete 1")
            self.send_command("display")

    def test_break_and_resume(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        for i in range(self.repeat_time):
            self.send_command("resume")
        for i in breakpoint_lines:
            self.send_command("delete 1")

    def test_step_into(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = ["page1.ets 22", "page2.ets 22", "page3.ets 25"]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-into")
        for i in range(self.repeat_time):
            self.send_command("resume")
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")
        
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)

        for i in range(self.repeat_time):
            self.send_command("step-into")
        for i in range(self.repeat_time):
            self.send_command("resume")
        
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)

        for i in range(self.repeat_time):
            self.send_command("step-into")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")
        
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_step_out(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("step-out")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("step-out")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("step-out")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_step_over(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = ["page1.ets 22", "page2.ets 22", "page3.ets 25"]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-over")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-over")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-over")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_step_into_and_step_out(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = ["page1.ets 22", "page2.ets 22", "page3.ets 25"]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-into")
            self.send_command("step-over")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-into")
            self.send_command("step-over")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-into")
            self.send_command("step-over")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_watch(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("watch a")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_repeat_watch(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = ["page1.ets 22", "page2.ets 22", "page3.ets 25"]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-over")
            self.send_command("watch a")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-over")
            self.send_command("watch a")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("step-over")
            self.send_command("watch a")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run("%s shell wukong special -t 600,600 -c 1" % self.hdctool, stdout=subprocess.PIPE)
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_print(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("print")
        self.send_command("print 2")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_repeat_print(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("print")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_cpuprofile(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("cpuprofile-enable")
            self.send_command("cpuprofile")
            self.send_command("step-over")
            self.send_command("cpuprofile-stop")
            self.send_command("cpuprofile-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_heapdump(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("heapprofiler-enable")
        self.send_command("heapdump")
        self.send_command("resume")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        self.send_command("heapprofiler-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")

        for i in breakpoint_lines:
            self.send_command("delete 1")
    
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_allocationtrack(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("heapprofiler-enable")
        self.send_command("allocationtrack")
        self.send_command("resume")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        self.send_command("allocationtrack-stop")
        self.send_command("heapprofiler-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")
        for i in breakpoint_lines:
            self.send_command("delete 1")
    
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_sampling(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("heapprofiler-enable")
            self.send_command("sampling")
            self.send_command("step-over")
            self.send_command("sampling-stop")
            self.send_command("heapprofiler-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")
        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_heapusage(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("heapusage")
        for i in range(self.repeat_time):
            self.send_command("resume")
        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_showstack(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("showstack")
        self.send_command("resume")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_combined_cmds(self):
        if (os.environ.get("isRelease", "") == "True" or os.environ.get("isSkipped", "") == "True"):
            return
        breakpoint_lines = [
            "page1.ets 22", "page2.ets 22", "page3.ets 25"
            ]
        self.send_command("rt-enable")
        for line in breakpoint_lines:
            self.send_command("b entry/src/main/ets/pages/%s" % line)
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )

        self.send_command("step-into")
        self.send_command("watch")
        self.send_command("step-out")
        self.send_command("print")
        self.send_command("heapprofiler-enable")
        self.send_command("sampling")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        self.send_command("sampling-stop")
        self.send_command("heapprofiler-disable")
        self.send_command("step-over")
        self.send_command("cpuprofile-enable")
        self.send_command("cpuprofile")
        self.send_command("cpuprofile-stop")
        self.send_command("cpuprofile-disable")
        self.send_command("heapusage")
        self.send_command("showstack")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        for i in breakpoint_lines:
            self.send_command("delete 1")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_only_cpuprofile(self):
        if (os.environ.get("isSkipped", "") == "True"):
            return
        self.send_command("rt-enable")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("cpuprofile-enable")
            self.send_command("cpuprofile")
            self.send_command("cpuprofile-stop")
            self.send_command("cpuprofile-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_only_heapdump(self):
        if (os.environ.get("isSkipped", "") == "True"):
            return
        self.send_command("rt-enable")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("heapprofiler-enable")
        self.send_command("heapdump")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        self.send_command("heapprofiler-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")
    
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_only_allocationtrack(self):
        if (os.environ.get("isSkipped", "") == "True"):
            return
        self.send_command("rt-enable")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("heapprofiler-enable")
        self.send_command("allocationtrack")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
        self.send_command("allocationtrack-stop")
        self.send_command("heapprofiler-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")
    
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_only_sampling(self):
        if (os.environ.get("isSkipped", "") == "True"):
            return
        self.send_command("rt-enable")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("heapprofiler-enable")
            self.send_command("sampling")
            self.send_command("sampling-stop")
            self.send_command("heapprofiler-disable")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_only_heapusage(self):
        if (os.environ.get("isSkipped", "") == "True"):
            return
        self.send_command("rt-enable")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("heapusage")
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

    def test_only_showstack(self):
        if (os.environ.get("isSkipped", "") == "True"):
            return
        self.send_command("rt-enable")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        self.send_command("showstack")
        self.send_command("resume")
        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")

        subprocess.run(
            "%s shell wukong special -t 600,600 -c 1" % self.hdctool,
            stdout=subprocess.PIPE
        )
        for i in range(self.repeat_time):
            self.send_command("resume")
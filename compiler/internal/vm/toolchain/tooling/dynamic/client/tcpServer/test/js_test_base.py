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

import os
import socket
import time
import subprocess
import sys
import unittest


class JsTestBase(unittest.TestCase):
    hdctool = None
    tcp_client_socket = None
    HOST = '127.0.0.1'
    PORT = 9999
    BUFSIZ = 1024
    ADDRESS = (HOST, PORT)

    def get_hdctool(self):
        self.hdctool = os.environ.get("HDCTool", "hdc")

    def connect_client_socket(self, dbg_name):
        self.tcp_client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        subprocess.run(
            [
                self.hdctool,
                "fport",
                "tcp:%i" % self.PORT,
                "tcp:%i" % self.PORT,
            ],
            stdout=subprocess.PIPE
        )
        connection = 0
        while connection < 5:
            arkdb_server = False
            ret = None
            result = subprocess.run(
                '%s shell "netstat -anp | grep PandaDebugger"' % os.environ.get("HDCTool", "hdc"),
                stdout=subprocess.PIPE,
            )
            if (result.returncode == 0):
                ret = result.stdout
            else:
                print("Failed to grep PandaDebugger!")
                sys.exit(0)

            out_str = str(ret).split('\\n')
            for sub_str in out_str:
                if (sub_str.find('CONNECTED') != -1):
                    arkdb_server = True
            if arkdb_server == True:
                self.tcp_client_socket.connect(self.ADDRESS)
                break
            time.sleep(1)
            connection = connection + 1
        if connection == 5:
            self.fail(self, "Not connect to arkdb server.")

    def close_client_socket(self):
        self.tcp_client_socket.close()

    def send_command(self, command):
        if command == '':
            self.fail('Command is not vaild')
        sent = self.tcp_client_socket.send(command.encode('utf-8'))
        if sent == 0:
            self.fail('Failed to send command: %s' % command)
        try:
            self.tcp_client_socket.settimeout(15)
            os.environ["isSkipped"] = "False"
            data, addr = self.tcp_client_socket.recvfrom(self.BUFSIZ)
            print("Recv: ", data.decode('utf-8'))
        except TimeoutError:
            self.tcp_client_socket.close()
            os.environ["isSkipped"] = "True"
            self.fail("TimeoutError")

    def run_arkdb_server(self, dbg_name):
        panda_dbg = []
        real_panda_name = None
        ret = None
        result = subprocess.run(
            '%s shell "netstat -anp | grep PandaDebugger"' % os.environ.get("HDCTool", "hdc"),
            stdout=subprocess.PIPE,
        )
        if (result.returncode == 0):
            ret = result.stdout
        else:
            print("Failed to grep PandaDebugger!")
            sys.exit(0)

        out_str = str(ret).split('\\n')
        pos_start = out_str[0].find("@") + 1
        out_str = out_str[0].split('\\r')[0]
        panda_dbg.append(out_str[pos_start :])
        if len(panda_dbg) == 0:
            print('Not exist PandaDebugger with name %s' % dbg_name)
            sys.exit(0)
        else:
            panda_name_len = 0
            for sub_panda in panda_dbg:
                if panda_name_len == 0:
                    panda_name_len = len(sub_panda)
                    real_panda_name = sub_panda
                elif len(sub_panda) < panda_name_len:
                    panda_name_len = len(sub_panda)
                    real_panda_name = sub_panda

        subprocess.Popen(
            [self.hdctool, 'shell', 'arkdb', real_panda_name, 'server'],
            stdout=subprocess.PIPE,
        )

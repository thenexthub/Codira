#!/usr/bin/env python
# coding=utf-8

# #
# # Copyright (c) 2024 Huawei Device Co., Ltd.
# # Licensed under the Apache License, Version 2.0 (the "License");
# # you may not use this file except in compliance with the License.
# # You may obtain a copy of the License at
# #
# #     http://www.apache.org/licenses/LICENSE-2.0
# #
# # Unless required by applicable law or agreed to in writing, software
# # distributed under the License is distributed on an "AS IS" BASIS,
# # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# # See the License for the specific language governing permissions and
# # limitations under the License.
# #

import argparse
import os
import sys
import unittest
import config


class RunTest:
    args = None
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    hap_list = []

    def __init__(self):
        self.parse_args()
        self.run_test()

    def parse_args(self):
        parser = argparse.ArgumentParser(description='Plugin test.')
        parser.add_argument(
            '-hdctool', 
            action='store',
            required=True,
            default=None,
            help='Remote connnection tool.'
        )
        self.args = parser.parse_args()

    def discover_cases(self):
        current_dir = os.path.dirname(os.path.abspath(__file__))
        case_dir = current_dir + os.sep + 'js_test_case'
        discover = self.loader.discover(case_dir, pattern='*.py')
        self.suite.addTest(discover)

    def get_panda_list(self):
        self.hap_list = config.hap_list

    def run_test(self):
        if self.args.hdctool is not None:
            os.environ["HDCTool"] = self.args.hdctool
        else:
            print("please input the path to hdc tool!")
            sys.exit(0)
        self.get_panda_list()
        for hap_name in self.hap_list:
            if (hap_name != "com.example.myapplication1"):
                os.environ["isRelease"] = "False"
            else:
                os.environ["isRelease"] = "True"
            self.suite = unittest.TestSuite()
            self.discover_cases()
            os.environ["HapName"] = hap_name
            runner = unittest.TextTestRunner(verbosity=2)
            runner.run(self.suite)

Main = RunTest

if __name__ == '__main__':
    Main()

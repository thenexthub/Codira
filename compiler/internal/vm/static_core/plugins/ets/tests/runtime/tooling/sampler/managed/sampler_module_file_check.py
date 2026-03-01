#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Copyright (c) 2022-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
import sys
import argparse
import logging
import numpy as np


class ModulesDumpTest:
    _file_name: str
    _binary_dir: str
    _checksum_name_map: dict

    def __init__(self, args) -> None:
        self._file_name = args.file
        self._binary_dir = args.bindir
        self._checksum_name_map = dict()

    @staticmethod
    def parse_args():
        parser = argparse.ArgumentParser(description="Sampler module file check test")
        required = parser.add_argument_group("required arguments")
        required.add_argument("--file", type=str, required=True)
        required.add_argument("--bindir", type=str, required=True)

        args = parser.parse_args()

        return args

    @staticmethod
    def get_checksum_from_abc_file(module_path) -> np.uint32:
        checksum_offset = 8 # bytes

        with open(module_path, mode="r+") as file:
            checksum = np.fromfile(file, dtype=np.uint32, count=1, sep='', offset=checksum_offset)
            return checksum[0]

    def fill_map(self, modules_list: list):
        for module in modules_list:
            checksum = self.get_checksum_from_abc_file(module)
            self._checksum_name_map[module] = checksum

    def module_file_check(self) -> bool:
        etsstdlib_abc_path = os.path.join(self._binary_dir, "plugins", "ets", "etsstdlib.abc")
        sampler_test_abc_path = os.path.join(self._binary_dir,
                                             "plugins",
                                             "ets",
                                             "tests",
                                             "runtime",
                                             "tooling",
                                             "sampler",
                                             "managed",
                                             "SamplerTest.abc")

        etsstdlib_abc_path = os.path.realpath(etsstdlib_abc_path)
        sampler_test_abc_path = os.path.realpath(sampler_test_abc_path)

        modules_list = [etsstdlib_abc_path, sampler_test_abc_path]

        self.fill_map(modules_list)

        modules_from_file = []

        with open(self._file_name, "r") as my_file:
            while line := my_file.readline():
                line = line.rstrip("\n")
                line_content = line.split(" ")

                pathname: str = os.path.realpath(line_content[1])
                modules_from_file.append(pathname)

                checksum: np.uint32 = np.uint32(line_content[0])
                expected_checksum: np.uint32 | None = self._checksum_name_map.get(pathname, None)

                if (expected_checksum is None):
                    logging.error("sampler_module_file_check: can not find expected checksum for ", pathname)
                    return False

                if (checksum != expected_checksum):
                    logging.error("sampler_module_file_check: for file",
                          pathname,
                          "checksum is not equal",
                          expected_checksum,
                          "vs",
                          checksum)
                    return False

        if (len(modules_from_file) != len(modules_list)):
            error_message = (
                "sampler_module_file_check: wrong number of modules in module file: "
                f"expected {len(modules_list)}, in file {len(modules_from_file)}"
            )
            logging.error(error_message)
            return False

        return True


def main():
    args = ModulesDumpTest.parse_args()
    test = ModulesDumpTest(args)

    if not test.module_file_check():
        logging.error("sampler_module_file_check: test failed")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())

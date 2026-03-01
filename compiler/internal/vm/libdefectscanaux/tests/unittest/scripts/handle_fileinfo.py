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

Description: Process fileinfo file, modify the first path to a local absolute path
"""

import sys
import os


def add_path_to_file(input_file, output_file, prefix_path):
    fd = os.open(output_file, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    with os.fdopen(fd, 'w') as outfile, open(input_file, 'r') as infile:
        for line in infile:
            outfile.write(f"{prefix_path}{line}")


def main(input_file, output_file, prefix_path):
    add_path_to_file(input_file, output_file, prefix_path)


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <input_file> <output_file> <prefix_path>")
        sys.exit(1)
    main(sys.argv[1], sys.argv[2], sys.argv[3])

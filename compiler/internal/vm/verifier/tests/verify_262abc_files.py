#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

import os
import subprocess
import argparse
import time
from datetime import datetime


def verify_abc_files(folder_path, verifier_path):
    total_count = 0
    success_count = 0
    failure_count = 0

    start_time = time.time()

    for root, _, files in os.walk(folder_path):
        for file in files:
            if not file.endswith('.abc'):
                continue
            total_count += 1
            file_path = os.path.join(root, file)
            command = [verifier_path, "--input_file", file_path]
            result = subprocess.run(command, shell=False)
            
            if result.returncode == 0:
                success_count += 1
                print(f"Success: {file_path}")
            else:
                failure_count += 1
                print(f"Failure: {file_path}")

    end_time = time.time()
    elapsed_time = end_time - start_time
    completion_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    print(f"Total .abc files: {total_count}")
    print(f"Success: {success_count}")
    print(f"Failure: {failure_count}")
    print(f"Execution time: {elapsed_time:.2f} seconds")
    print(f"Completion time: {completion_time}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Verify .abc files using ark_verifier tool.")
    parser.add_argument("--folder_path", type=str, required=True, help="Path to the folder containing .abc files")
    parser.add_argument("--verifier_path", type=str, required=True, help="Path to the ark_verifier tool")
    
    args = parser.parse_args()
    
    verify_abc_files(args.folder_path, args.verifier_path)

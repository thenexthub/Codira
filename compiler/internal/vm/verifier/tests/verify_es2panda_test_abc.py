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

import argparse
import os
import shutil
import subprocess
import time


def parse_args():
    parser = argparse.ArgumentParser(description="Generate and verify ABC files.")
    parser.add_argument(
        "--es2abc-dir", required=True, help="Path to the es2abc directory.")
    parser.add_argument(
        "--verifier-dir", required=True, help="Path to the ark_verifier directory.")
    parser.add_argument(
        "--js-ts-dir", required=False, help="Path to the directory containing .js and .ts files.")
    parser.add_argument(
        "--keep-files", action="store_true", help="Keep generated files after verification.")
    return parser.parse_args()


def find_files(root_dir, extensions):
    for root, _, files in os.walk(root_dir):
        for file in files:
            if file.endswith(extensions):
                yield os.path.join(root, file)


def copy_files(src_dir, dest_dir, extensions):
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    
    for file_path in find_files(src_dir, extensions):
        dest_path = os.path.join(dest_dir, os.path.basename(file_path))
        shutil.copy(file_path, dest_path)


def setup_directories(script_dir):
    temp_files_dir = os.path.join(script_dir, "temp_files")
    temp_js_ts_dir = os.path.join(temp_files_dir, "temp_js_ts")
    temp_abc_dir = os.path.join(temp_files_dir, "temp_abc")

    if os.path.exists(temp_files_dir):
        shutil.rmtree(temp_files_dir)

    os.makedirs(temp_js_ts_dir, exist_ok=True)
    os.makedirs(temp_abc_dir, exist_ok=True)

    return temp_files_dir, temp_js_ts_dir, temp_abc_dir


def process_file(file_path, es2abc_path, temp_abc_dir, verifier_dir):
    base_name = os.path.splitext(os.path.basename(file_path))[0]
    abc_filename = f"{base_name}.abc"
    abc_output_path = os.path.join(temp_abc_dir, abc_filename)
    es2abc_command = [es2abc_path, "--module", file_path, "--output", abc_output_path]

    stdout, stderr, returncode = run_command(es2abc_command)
    if returncode != 0:
        print(f"Ignored {file_path} (Failed to generate ABC)")
        return False, True

    verifier_command = [os.path.join(verifier_dir, "ark_verifier"), "--input_file", abc_output_path]
    stdout, stderr, returncode = run_command(verifier_command)
    if returncode == 0:
        return True, False
    else:
        print(f"Verification failed for {abc_output_path}:\n{stderr}")
        return False, False


def run_command(command):
    result = subprocess.run(command, capture_output=True, text=True)
    return result.stdout, result.stderr, result.returncode


def report_results(total, passed, failed, ignored, start_time):
    end_time = time.time()
    duration = end_time - start_time
    completion_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(end_time))

    print(f"Total JS/TS files: {total}")
    print(f"Ignored JS/TS files: {ignored}")
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    print(f"Execution time: {duration:.2f} seconds")
    print(f"Completion time: {completion_time}")


def main():
    start_time = time.time()
    args = parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    temp_files_dir, temp_js_ts_dir, temp_abc_dir = setup_directories(script_dir)

    js_ts_dir = args.js_ts_dir if args.js_ts_dir else os.path.join(script_dir, "../../../ets_frontend/es2panda/test/")
    copy_files(js_ts_dir, temp_js_ts_dir, (".js", ".ts"))
    
    total = 0
    passed = 0
    failed = 0
    ignored = 0

    js_ts_files = list(find_files(temp_js_ts_dir, (".js", ".ts")))
    es2abc_path = os.path.join(args.es2abc_dir, "es2abc")

    for file_path in js_ts_files:
        total += 1
        success, ignore = process_file(file_path, es2abc_path, temp_abc_dir, args.verifier_dir)
        if ignore:
            ignored += 1
        elif success:
            passed += 1
        else:
            failed += 1

    if not args.keep_files:
        if os.path.isdir(temp_files_dir):
            shutil.rmtree(temp_files_dir)
            print(f"\n'{temp_files_dir}' directory has been deleted.")

    report_results(total, passed, failed, ignored, start_time)


if __name__ == "__main__":
    main()

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
import datetime
import os
import subprocess
import shutil
from config import JS_FILE, API_VERSION_MAP, Color


def parse_args():
    parser = argparse.ArgumentParser(description="Version compatibility testing for Verifier.")
    parser.add_argument(
        "--verifier-dir", required=True, help="Path to the ark_verifier.")
    parser.add_argument(
        "--es2abc-dir", required=True, help="Path to the es2abc.")
    parser.add_argument(
        "--keep-files", action="store_true", help="Keep the diff_version_abcs directory.")
    return parser.parse_args()


def create_output_dir():
    output_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "diff_version_abcs")
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)
    return output_dir


def split_api_version(version_str):
    if not version_str.startswith("API"):
        raise ValueError(f"Invalid version string format: {version_str}")

    parts = version_str.split("API")[1].split("beta")
    main_part = parts[0]
    # Create "beta*" if there is a beta part; otherwise, set to empty string
    beta_part = f"beta{parts[1]}" if len(parts) > 1 else ""

    return (main_part, beta_part)


def compile_abc(es2abc_path, js_file, output_file, target_api_version, target_api_sub_version):
    cmd = [
        es2abc_path,
        "--module", js_file,
        "--output", output_file,
        f"--target-api-version={target_api_version}",
    ]

    if target_api_sub_version:
        cmd.append(f"--target-api-sub-version={target_api_sub_version}")

    result = subprocess.run(cmd, capture_output=True, text=True)

    print(f"Executing command: {' '.join(cmd)}")

    if result.returncode != 0:
        print(f"Error in compiling {output_file}: {result.stderr}")
        raise subprocess.CalledProcessError(result.returncode, cmd)
    else:
        print(f"Compiled {output_file}: {result.stdout}")


def verify_abcs(verifier_dir, output_dir):
    input_files = [os.path.join(output_dir, f) for f in os.listdir(output_dir) if f.endswith(".abc")]
    passed_count = 0
    failed_count = 0
    failed_files = []

    for abc_file in input_files:
        cmd = [os.path.join(verifier_dir, "ark_verifier"), "--input_file", abc_file]
        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode == 0:
            print(f"Verification passed for {abc_file}: {result.stdout}")
            passed_count += 1
        else:
            print(f"Verification failed for {abc_file}: {result.stderr}")
            failed_count += 1
            failed_files.append(abc_file)

    total_count = passed_count + failed_count
    return passed_count, failed_count, total_count, failed_files


def report_results(passed, failed, total, failed_files):
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    print(Color.apply(Color.WHITE, f"Total ABC files: {total}"))
    print(Color.apply(Color.GREEN, f"Passed: {passed}"))
    print(Color.apply(Color.RED, f"Failed: {failed}"))
    print(f"Report generated at: {timestamp}")
    if failed > 0:
        print("Failed files:")
        for file in failed_files:
            print(f"  - {file}")


def main():
    args = parse_args()

    verifier_dir = args.verifier_dir
    es2abc_dir = args.es2abc_dir

    script_dir = os.path.dirname(os.path.realpath(__file__))
    js_file = os.path.join(script_dir, "js", JS_FILE)
    
    if not os.path.exists(js_file):
        raise FileNotFoundError(f"{js_file} not found.")

    output_dir = create_output_dir()

    es2abc_path = os.path.join(es2abc_dir, "es2abc")
    if not os.path.exists(es2abc_path):
        raise FileNotFoundError(f"{es2abc_path} not found.")

    for api_key in API_VERSION_MAP.keys():
        target_api_version, target_api_sub_version = split_api_version(api_key)

        output_file = os.path.join(output_dir, f"{api_key}.abc")
        compile_abc(
            es2abc_path, js_file, output_file, target_api_version, 
            target_api_sub_version if target_api_version == "12" else ""
        )

    passed, failed, total, failed_files = verify_abcs(verifier_dir, output_dir)
    report_results(passed, failed, total, failed_files)

    if failed == 0 and not args.keep_files:
        shutil.rmtree(output_dir)
        print(f"Deleted {output_dir} as all verifications passed.")
    else:
        print(f"Verification failed or keep-files flag is set, keeping {output_dir} for inspection.")


if __name__ == "__main__":
    main()

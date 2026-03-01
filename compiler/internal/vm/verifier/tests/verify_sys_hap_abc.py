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
import shutil
import stat
import subprocess
import time
import zipfile


def parse_args():
    parser = argparse.ArgumentParser(description="Verify abc files in system app.")
    parser.add_argument(
        "--hap-dir", required=True, help="Path to the HAP files directory.")
    parser.add_argument(
        "--verifier-dir", required=True, help="Path to the ark_verifier directory.")
    parser.add_argument(
        "--keep-files", action="store_true", help="Keep extracted files after verification.")
    return parser.parse_args()


def copy_and_rename_hap_files(hap_folder, out_folder):
    for file_path in os.listdir(hap_folder):
        if file_path.endswith(".hap"):
            destination_path = os.path.join(out_folder, file_path.replace(".hap", ".zip"))
            shutil.copy(os.path.join(hap_folder, file_path), destination_path)


def extract_zip(zip_path, extract_folder):
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_folder)
    except zipfile.BadZipFile as e:
        print(f"Error extracting {zip_path}: {e}")


def verify_file(file_path, ark_verifier_path):
    verification_command = ["/usr/bin/time", "-v", ark_verifier_path, "--input_file", file_path]
    result = subprocess.run(verification_command, capture_output=True, text=True)
    status = 'pass' if result.returncode == 0 else 'fail'

    memory_usage = None
    user_time_ms = None

    for line in result.stderr.splitlines():
        if "Maximum resident set size" in line:
            memory_usage = f"{line.split(':')[1].strip()} KB"
        if "User time (seconds)" in line:
            user_time_seconds = float(line.split(":")[1].strip())
            user_time_ms = f"{user_time_seconds * 1000:.2f} ms"

    file_size = os.path.getsize(file_path)
    file_size_str = f"{file_size / 1024:.2f} KB" if file_size < 1024**2 else f"{file_size / 1024**2:.2f} MB"
    report = {
        "file": file_path,
        "size": file_size_str,
        "status": status,
        "memory_usage": memory_usage,
        "user_time": user_time_ms,
    }
    return report


def process_directory(directory, ark_verifier_path):
    total_count = 0
    passed_count = 0
    failed_abc_list = []
    report_list = []

    for root, dirs, files in os.walk(directory):
        for file in files:
            if not file.endswith(".abc"):
                continue
            abc_path = os.path.join(root, file)
            print(f"Verifying file: {abc_path}")
            report = verify_file(abc_path, ark_verifier_path)
            report_list.append(report)
            if report.get("status") == "pass":
                passed_count += 1
            else:
                failed_abc_list.append(os.path.relpath(abc_path, directory))
            total_count += 1

    return total_count, passed_count, failed_abc_list, report_list


def verify_hap(hap_folder, ark_verifier_path):
    failed_abc_list = []
    passed_count = 0
    total_count = 0
    report_list = []

    for file in os.listdir(hap_folder):
        if not file.endswith(".zip"):
            continue

        zip_path = os.path.join(hap_folder, file)
        extract_folder = os.path.join(hap_folder, file.replace(".zip", ""))
        print(f"Extracting {zip_path} to {extract_folder}")
        extract_zip(zip_path, extract_folder)

        ets_path = os.path.join(extract_folder, "ets")
        if not os.path.exists(ets_path):
            continue

        modules_abc_path = os.path.join(ets_path, "modules.abc")
        if os.path.isfile(modules_abc_path):
            print(f"Verifying file: {modules_abc_path}")
            report = verify_file(modules_abc_path, ark_verifier_path)
            report_list.append(report)
            if report.get("status") == "pass":
                passed_count += 1
            else:
                failed_abc_list.append(os.path.relpath(modules_abc_path, hap_folder))
            total_count += 1
        else:
            total_inc, passed_inc, failed_abc_inc, reports = process_directory(ets_path, ark_verifier_path)
            total_count += total_inc
            passed_count += passed_inc
            failed_abc_list.extend(failed_abc_inc)
            report_list.extend(reports)

    return total_count, passed_count, len(failed_abc_list), failed_abc_list, report_list


def save_report(report_list, report_file):
    flags = os.O_RDWR | os.O_CREAT
    mode = stat.S_IWUSR | stat.S_IRUSR
    with os.fdopen(os.open(report_file, flags, mode), 'w') as f:
        f.truncate()
        f.write("<html><head><title>Verification Report</title>")
        f.write("<style>")
        f.write("body {font-family: Arial, sans-serif;}")
        f.write("table {width: 100%; border-collapse: collapse;}")
        f.write("th, td {border: 1px solid black; padding: 8px; text-align: left;}")
        f.write("th {background-color: #f2f2f2;}")
        f.write("tr:nth-child(even) {background-color: #f9f9f9;}")
        f.write("tr:hover {background-color: #f1f1f1;}")
        f.write("</style></head><body>\n")
        f.write("<h1>Verification Report</h1>\n")
        f.write(f"<p>Generated on: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>\n")
        f.write("<table>\n")
        f.write("<tr><th>File</th><th>Size</th><th>Status</th><th>Memory Usage</th><th>User Time</th></tr>\n")
        for report in report_list:
            f.write("<tr>")
            f.write(f"<td>{report['file']}</td>")
            f.write(f"<td>{report['size']}</td>")
            f.write(f"<td>{report['status']}</td>")
            f.write(f"<td>{report['memory_usage']}</td>")
            f.write(f"<td>{report['user_time']}</td>")
            f.write("</tr>\n")
        f.write("</table>\n")
        f.write("</body></html>\n")


def main():
    start_time = time.time()
    args = parse_args()

    hap_folder_path = os.path.abspath(args.hap_dir)
    ark_verifier_path = os.path.abspath(os.path.join(args.verifier_dir, "ark_verifier"))

    script_dir = os.path.dirname(os.path.abspath(__file__))
    out_folder = os.path.join(script_dir, "out")
    os.makedirs(out_folder, exist_ok=True)

    copy_and_rename_hap_files(hap_folder_path, out_folder)

    total_count, passed_count, failed_count, failed_abc_list, report_list = verify_hap(out_folder, ark_verifier_path)

    print("Summary(abc verification):")
    print(f"Total: {total_count}")
    print(f"Passed: {passed_count}")
    print(f"Failed: {failed_count}")

    if failed_count > 0:
        print("\nFailed abc files:")
        for failed_abc in failed_abc_list:
            print(f"  - {failed_abc}")

    report_file = os.path.join(script_dir, "verification_report.html")
    save_report(report_list, report_file)
    print(f"\nDetailed report saved to: {report_file}")

    if not args.keep_files:
        if os.path.isdir(out_folder):
            shutil.rmtree(out_folder)
            print(f"\n'{out_folder}' directory has been deleted.")

    end_time = time.time()
    duration = end_time - start_time
    completion_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(end_time))

    print(f"\nExecution time: {duration:.2f} seconds")
    print(f"Completion time: {completion_time}")


if __name__ == "__main__":
    main()

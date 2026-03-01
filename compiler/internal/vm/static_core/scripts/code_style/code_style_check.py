#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
import multiprocessing
import re
import sys
import subprocess
import time


CLANG_FORMAT = "clang-format-14"


def get_args():
    parser = argparse.ArgumentParser(
        description="Runner for clang-format for panda project.")
    parser.add_argument(
        'panda_dir', help='panda sources directory.')
    parser.add_argument(
        '--reformat', action="store_true", help='reformat files.')
    parser.add_argument(
        '--proc-count', type=int, action='store', dest='proc_count',
        required=False, default="-1",
        help='Paralell process count of clang-format')
    return parser.parse_args()


def run_clang_format(src_path, panda_dir, reformat, msg):
    check_cmd = [str(os.path.join(panda_dir, 'scripts', 'code_style',
                                  'run_code_style_tools.sh'))]
    reformat_cmd = [CLANG_FORMAT, '-i']
    cmd = reformat_cmd if reformat else check_cmd
    cmd += [src_path]

    print(msg)

    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        # Skip error for some invalid release configurations.
        if not e.stdout:
            print("Note: missed output for ", src_path)
            return True

        print('Failed: ', ' '.join(cmd))
        print(e.stdout.decode())

        if e.stderr:
            print(e.stderr.decode())

        return False

    return True


def get_proc_count(cmd_ard : int) -> int:
    if cmd_ard > 0:
        return cmd_ard

    min_proc_str = os.getenv('NPROC_PER_JOB')
    if min_proc_str:
        return int(min_proc_str)

    return multiprocessing.cpu_count()


def check_file_list(file_list : list, panda_dir : str, reformat : bool, proc_count : int):
    pool = multiprocessing.Pool(proc_count)
    jobs = []
    main_ret_val = True
    total_count = str(len(file_list))
    idx = 0
    for src in file_list:
        idx += 1
        msg = "[%s/%s] Running clang-format: %s" % (str(idx), total_count, src)
        proc = pool.apply_async(func=run_clang_format, args=(
            src, panda_dir, reformat, msg))
        jobs.append(proc)

    # Wait for jobs to complete before exiting
    while(not all([p.ready() for p in jobs])):
        time.sleep(5)

    for job in jobs:
        if not job.get():
            main_ret_val = False
            break

    # Safely terminate the pool
    pool.close()
    pool.join()

    return main_ret_val


def get_file_list(panda_dir):
    src_exts = (".c", '.cc', ".cp", ".cxx", ".cpp", ".CPP", ".c++", ".C", ".h",
                ".hh", ".H", ".hp", ".hxx", ".hpp", ".HPP", ".h++", ".tcc", ".inc")
    skip_dirs = ["third_party", "artifacts", "build.*"]
    file_list = []
    for dirpath, dirnames, filenames in os.walk(panda_dir, followlinks=True):
        dirnames[:] = [d for d in dirnames if not re.match(f"({')|('.join(skip_dirs)})", d)]
        for fname in filenames:
            if (fname.endswith(src_exts)):
                full_path = os.path.join(panda_dir, dirpath, fname)
                full_path = str(os.path.realpath(full_path))
                file_list.append(full_path)

    return file_list


if __name__ == "__main__":
    args = get_args()
    files_list = []

    files_list = get_file_list(args.panda_dir)

    if not files_list:
        sys.exit(
            "Can't be prepaired source list. Please check panda_dir variable: " + args.panda_dir)

    process_count = get_proc_count(args.proc_count)
    print('clang-format proc_count: ' + str(process_count))

    if not check_file_list(files_list, args.panda_dir, args.reformat, process_count):
        sys.exit("Failed: clang-format get errors")

    print("Clang-format was passed successfully!")

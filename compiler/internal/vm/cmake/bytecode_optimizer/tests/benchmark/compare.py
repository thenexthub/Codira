#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

import argparse
import os
import json
import subprocess

SRC_PATH = os.path.realpath(os.path.dirname(__file__))

parser = argparse.ArgumentParser("Compare benchmark results")
parser.add_argument("--old", metavar="JSON_FILE_PATH", required=True,
                    help="Base or reference benchmark result")
parser.add_argument("--new", metavar="JSON_FILE_PATH", required=True,
                    help="Benchmark result to be compared with the reference")
parser.add_argument("--failed", metavar="JSON_FILE_PATH", required=False,
                    help="File to log error messages from c2p")


def exists(name, d):
    if name in d:
        return True
    return False


args = parser.parse_args()

if __name__ == '__main__':
    if not os.path.exists(args.old):
        print("Input file (%s) does not exists." % args.old)
        exit(1)

    if not os.path.exists(args.new):
        print("Input file (%s) does not exists." % args.new)
        exit(1)

    with open(args.old, 'r') as old_fp, open(args.new, 'r') as new_fp:
        old_res, new_res = json.load(old_fp), json.load(new_fp)

    result = {}
    failed_log_new = {}
    failed_log_old = {}
    flog = {}
    optimized_files = 0  # number of files that have been optimized
    not_optimized_files = 0  # files that are not optimized
    de_optimized_files = 0  # code_item section is bigger than baseline
    empty_files = 0  # files with no code_item section
    old_size = 0
    new_size = 0
    sum_old = 0
    sum_new = 0
    failed_old = 0
    failed_new = 0
    sum_optimized_old = 0
    sum_optimized_new = 0

    for filename in old_res.keys():
        if not exists("error", old_res[filename]):
            if not exists("error", new_res[filename]):
                if exists("code_item section", new_res[filename]):
                    old_size = old_res[filename]["code_item section"]
                    new_size = new_res[filename]["code_item section"]
                    sum_old += old_size
                    sum_new += new_size
                    diff = old_size - new_size
                    result[filename] = {"old_size": old_size,
                                        "new_size": new_size, "diff": diff}
                    if diff > 0:
                        optimized_files += 1
                        sum_optimized_old += old_size
                        sum_optimized_new += new_size
                    elif diff < 0:
                        de_optimized_files += 1
                    else:
                        not_optimized_files += 1
                else:
                    empty_files += 1
            else:
                failed_new += 1
                failed_log_new[filename] = new_res[filename]
        else:
            failed_old += 1
            failed_log_old[filename] = old_res[filename]

    print("Classes that have been optimized:\n  Code_item section size:\n|Old: |New: |Diff:|Per:  |File:")
    for r in result:
        if result[r]["diff"] > 0:
            print("|{:5d}|{:5d}|{:5d}|{:5.2f}%| {:s}".format(
                result[r]["old_size"],
                result[r]["new_size"],
                result[r]["diff"],
                100 * (1 - float(result[r]["new_size"]) / result[r]["old_size"]), r))

    print("""\nSummary:\n=============\
    \n Total code_item section size of baseline files: {:d} bytes\
    \n Total code_item section size of compared files: {:d} bytes\
    \n Difference: {:d} bytes [{:3.2f}%]\
    \n Number of optimized files: {:d}\
    \n Number of not optimized files : {:d}\
    \n Files with no code item section: {:d}\
    \n Files that are bigger than baseline: {:d}\
    \n Failed tests on baseline: {:d}\
    \n Failed tests compared to baseline: {:d}\
    \n============="""
          .format(sum_old, sum_new, sum_old - sum_new,
                  100 * (1 - float(sum_new) / sum_old) if sum_old != 0 else 0,
                  optimized_files, not_optimized_files, empty_files, de_optimized_files, failed_old, failed_new))

    print("""\nStatistics on optimized files:\n============= \
    \n Total code_item section size of baseline files: {:d} bytes\
    \n Total code_item section size of compared files: {:d} bytes\
    \n Difference: {:d} bytes [{:3.2f}%]\
    \n============="""
          .format(sum_optimized_old, sum_optimized_new,
                  sum_optimized_old - sum_optimized_new,
                  100 * (1 - float(sum_optimized_new) / sum_optimized_old) if sum_optimized_old != 0 else 0))

    if args.failed:
        with open(args.failed, 'w') as fp:
            flog = {"Errors on baseline tests": failed_log_old,
                    "Errors on compared tests": failed_log_new}

            json.dump(flog, fp, indent=4)
            fp.write("\n")

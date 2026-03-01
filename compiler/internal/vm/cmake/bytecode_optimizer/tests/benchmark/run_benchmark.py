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
import tempfile

SRC_PATH = os.path.realpath(os.path.dirname(__file__))
testdir = os.path.join(SRC_PATH, "suite")
bindir = os.path.join(SRC_PATH, "..", "..", "..", "build", "bin")

parser = argparse.ArgumentParser("Run bytecode benchmarks")
parser.add_argument("--testdir", default=testdir,
                    help="Directory with tests (*.class or *.pa). Default: './%s'" % os.path.relpath(testdir))
parser.add_argument("--bindir", default=bindir,
                    help="Directory with compiled binaries (eg.: c2p). Default: './%s'" % os.path.relpath(bindir))
parser.add_argument("--input-type", choices=["class", "pa"], default="class",
                    help="Type of the test input. Default: '%(default)s'"),
parser.add_argument("--compiler-options", metavar="LIST",
                    help="Comma separated list of compiler options for C2P (see '%s --help' for details"
                    % (os.path.relpath(os.path.join(bindir, "c2p")))),
parser.add_argument("--compiler-options-file", metavar="FILE",
                    help="Input file containing compiler options for C2P (see '%s --help' for details"
                    % (os.path.relpath(os.path.join(bindir, "c2p")))),
parser.add_argument("--json", metavar="FILE",
                    help="JSON dump file name"),
parser.add_argument("--verbose", "-v", action='store_true',
                    help="Enable verbose messages")

args = parser.parse_args()


def log(msg):
    if args.verbose:
        print(msg)


def parse_c2p_output(name, stdout, stderr, returncode, d):
    d[name] = {}  # {'name': name: {'code_item section': 11}, {'total': 123}, ..}
    result = {}  # { 'name': name }
    # TODO: Handle segmentation fault correctly
    if returncode != 0:
        d[name] = {"error": stderr.decode('ascii')}
    else:
        for stdout_line in stdout.decode('ascii').split("\n"):
            if not stdout_line:
                continue
            key, value = stdout_line.split(":")[:2]
            if value:
                result[key] = int(value.strip())
                d[name][key] = int(value.strip())
    return result


def print_sizes_dict(sizes):
    for d in sorted(sizes):
        print("%28s: %d" % (d, sizes[d]))


def calc_statistics(sizes):
    total = len(sizes)
    if total == 0:
        return None
    keys = sizes[0].keys()
    avgs = dict.fromkeys(keys, 0)
    mins = dict.fromkeys(keys, 99999999)
    maxs = dict.fromkeys(keys, 0)
    for d in sizes:
        for k in keys:
            if k not in d:
                continue
            avgs[k] += d[k]

            if d[k] < mins[k]:
                mins[k] = d[k]
            if d[k] > maxs[k]:
                maxs[k] = d[k]

    for d in avgs:
        avgs[d] = round(avgs[d] / total, 1)

    print("\nAverage sizes (in bytes):")
    print_sizes_dict(avgs)
    print("\nMinimum sizes (in bytes):")
    print_sizes_dict(mins)
    print("\nMaximum sizes (in bytes):")
    print_sizes_dict(maxs)

    return {"Average sizes (in bytes)": avgs,
            "Minimum sizes (in bytes)": mins,
            "Maximum sizes (in bytes)": maxs}


def run_c2p(test_dir, bin_dir, c2p_opts):
    sizes = []
    result = {}
    tests_passed = 0
    tests_failed = 0

    c2p = os.path.join(bin_dir, "c2p")
    if not os.path.exists(c2p):
        print("c2p executable does not exists (%s)." % os.path.relpath(c2p))
        exit(2)
    fp = tempfile.NamedTemporaryFile()
    for dirpath, dirnames, filenames in os.walk(test_dir):
        for name in filenames:
            if name.endswith(".class"):
                test_path = os.path.join(dirpath, name)
                proc = subprocess.Popen([c2p, "--size-stat"] + c2p_opts + [
                                        test_path, fp.name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = proc.communicate()
                sizes.append(parse_c2p_output(test_path, stdout,
                             stderr, proc.returncode, result))
                if proc.returncode == 0:
                    tests_passed += 1
                else:
                    tests_failed += 1
                    log("Could not process the class file (%s)." % test_path)
                    pass
    fp.close()
    return sizes, tests_passed, tests_failed, result


############################################

if __name__ == '__main__':
    if not os.path.exists(args.testdir):
        print("Test directory (%s) does not exists." % args.testdir)
        exit(1)

    num_of_tests = 0
    passed_tests = 0
    failed_tests = 0

    if args.input_type == "class":
        c2p_options = []
        if args.compiler_options_file:
            with open(args.compiler_options_file) as conf_fp:
                lines = conf_fp.readlines()
                for line in lines:
                    c2p_options.append(line.strip())

        if args.compiler_options:
            c2p_options += args.compiler_options.split(",")

        c2p_sizes, passed_tests, failed_tests, c2p_res = run_c2p(
            args.testdir, args.bindir, c2p_options)
        num_of_tests = passed_tests + failed_tests
        stats = calc_statistics(c2p_sizes)
        if args.json:
            with open(args.json, 'w') as json_file:
                json.dump(c2p_res, json_file, indent=4, sort_keys=True)
                json_file.write("\n")

    else:
        print("Non class input types have not been implemented.")
        exit(2)

    print("Summary:\n========\n  Tests : %d\n  Passed: %d\n  Failed: %d\n"
          % (num_of_tests, passed_tests, failed_tests))

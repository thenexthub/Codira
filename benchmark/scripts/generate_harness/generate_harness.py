#!/usr/bin/env python3

# ===--- generate_harness.py ---------------------------------------------===//
#
# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli
#
# ===---------------------------------------------------------------------===//

# Generate boilerplate, CMakeLists.txt and utils/main.code from templates.

import argparse
import os
import subprocess

script_dir = os.path.dirname(os.path.realpath(__file__))
perf_dir = os.path.realpath(os.path.join(script_dir, "../.."))
gyb = os.path.realpath(os.path.join(perf_dir, "../utils/gyb"))
parser = argparse.ArgumentParser()
parser.add_argument(
    "--output-dir", help="Output directory (for validation test)", default=perf_dir
)
args = parser.parse_args()
output_dir = args.output_dir


def all_files(directory, extension):  # matching: [directory]/**/*[extension]
    return [
        os.path.join(root, f)
        for root, _, files in os.walk(directory)
        for f in files
        if f.endswith(extension)
    ]


def will_write(filename):  # ensure path to file exists before writing
    print(filename)
    output_path = os.path.split(filename)[0]
    if not os.path.exists(output_path):
        os.makedirs(output_path)


if __name__ == "__main__":
    # Generate Your Boilerplate
    # Make sure longer paths are done first as CMakeLists.txt and main.code
    # depend on the other gybs being generated first.
    gyb_files = sorted(all_files(perf_dir, ".gyb"), key=len, reverse=True)
    for f in gyb_files:
        relative_path = os.path.relpath(f[:-4], perf_dir)
        out_file = os.path.join(output_dir, relative_path)
        will_write(out_file)
        subprocess.call([gyb, "--line-directive", "", "-o", out_file, f])

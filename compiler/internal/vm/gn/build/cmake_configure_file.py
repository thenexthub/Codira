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

import re
import sys
import os
import argparse


def main():
    parser = argparse.ArgumentParser(
        description="Imitates functionality of cmake's configure_file")
    parser.add_argument("input", help="file to configure")
    parser.add_argument("output", help="name of the configured file")
    parser.add_argument(
        "values", help="words to match, format: KEY=VALUE or KEY=", nargs='*')

    args = parser.parse_args()

    # supports both @...@ and ${...} notations
    pattern = r'@([^@]*)@|\$\{([^}]*)\}'

    # parsed variables
    values = {}
    for value in args.values:
        key, val = value.split('=', 1)
        if key in values:
            return 1
        values[key] = val.replace('\\n', '\n')

    with open(args.input) as file:
        in_lines = file.readlines()
    out_lines = []
    for in_line in in_lines:
        def replace(pattern):
            key = pattern.group(1) or pattern.group(2)
            if key in values:
                return values[key]
            else:
                return ""

        in_line = re.sub(pattern, replace, in_line)

        # structure: #cmakedefine var val
        if in_line.startswith("#cmakedefine "):
            var_val = in_line.split(' ', 1)[1]
            var_val_split = var_val.split(' ', 1)
            if len(var_val_split) == 1:
                # only var given
                var = var_val.rstrip()
                in_line = '#define %s\n' % var
            else:
                var = var_val_split[0]
                val = var_val_split[1]
                in_line = '#define %s %s\n' % (var, val)

            if var not in values:
                in_line = '/* #undef %s */\n' % var

        out_lines.append(in_line)

    output = ''.join(out_lines)

    def read(filename):
        with open(filename) as file:
            return file.read()

    if not os.path.exists(args.output) or read(args.output) != output:
        with open(args.output, 'w') as file:
            file.write(output)
        os.chmod(args.output, os.stat(args.input).st_mode & 0o777)


if __name__ == '__main__':
    sys.exit(main())

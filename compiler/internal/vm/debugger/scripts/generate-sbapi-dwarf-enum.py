#!/usr/bin/env python3

import argparse
import re
import os

HEADER = """\
//===-- SBLanguages.h -----------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_API_SBLANGUAGE_H
#define LLDB_API_SBLANGUAGE_H

#include <cstdint>

namespace lldb {
/// Used by \\ref SBExpressionOptions.
/// These enumerations use the same language enumerations as the DWARF
/// specification for ease of use and consistency.
enum SBSourceLanguageName : uint16_t {
"""

FOOTER = """\
};

} // namespace lldb

#endif
"""

REGEX = re.compile(
    r'^ *HANDLE_DW_LNAME *\( *(?P<value>[^,]+), (?P<name>.*), "(?P<comment>[^"]+)",.*\)'
)


def emit_enum(input, output):
    # Read the input and break it up by lines.
    lines = []
    with open(input, "r") as f:
        lines = f.readlines()

    # Create output folder if it does not exist
    os.makedirs(os.path.dirname(output), exist_ok=True)

    # Write the output.
    with open(output, "w") as f:
        # Emit the header.
        f.write(HEADER)

        # Emit the enum values.
        for line in lines:
            match = REGEX.match(line)
            if not match:
                continue
            f.write(f"  /// {match.group('comment')}.\n")
            f.write(f"  eLanguageName{match.group('name')} = {match.group('value')},\n")

        # Emit the footer
        f.write(FOOTER)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", "-o")
    parser.add_argument("input")
    args = parser.parse_args()

    emit_enum(args.input, args.output)


if __name__ == "__main__":
    main()

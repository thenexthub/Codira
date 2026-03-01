#!/bin/bash
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

# arg1 - path to es2panda_build_info.h
# arg2 - path to es2panda_root

set -e

output_file="$1"
build_date=$(date "+%Y-%m-%d_%H:%M:%S")
build_hash=""
if git --version &> /dev/null
then
    if git -C "$2" rev-parse &> /dev/null
    then
        build_hash=$(git -C "$2" rev-parse HEAD)
    fi
fi
echo "#ifndef ES2PANDA_BUILD_INFO_H" > "$output_file"
echo "#define ES2PANDA_BUILD_INFO_H" >> "$output_file"
echo "#define ES2PANDA_DATE \"$build_date\"" >> "$output_file"
echo "#define ES2PANDA_HASH \"$build_hash\"" >> "$output_file"
echo "#endif" >> "$output_file"

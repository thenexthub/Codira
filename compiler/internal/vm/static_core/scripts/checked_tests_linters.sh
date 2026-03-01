#!/usr/bin/env bash
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

set -e

if [[ -z "$1" ]]; then
    echo "Usage: $0 <sources_dir>"
    echo "    <sources_dir> - path where the sources to be checked are located"
    exit 1
fi

sources_dir=$(realpath "$1")

EXIT_CODE=0

check_num=$(find "${sources_dir}"/plugins/ets/tests/checked -name "*.ets" | wc -l)
expected_num=5

if [[ $check_num -ne $expected_num ]]; then
    echo -e "Error: Number of checked tests in 'static_core/plugins/ets/tests/checked': expected ${expected_num} found: ${check_num}.
        Only tests with 'NATIVE' or 'JITINTERFACE' are permited to add.
        Other tests should be added into appropriate 'static_core/plugins/ets/tests/checked_urunner' folder." >&2
    EXIT_CODE=1
fi

exit ${EXIT_CODE}

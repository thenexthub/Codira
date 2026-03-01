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

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd -P)"
ARK_ROOT=$SCRIPT_DIR/../..
ARK_THIRD_PARTY_DIR=$ARK_ROOT/third_party

# In case of ARK standalone builds, all GN build files for third party
# components shoild be placed in $ARK_ROOT/gn/third_party directory,
# so we delete build files intended for the original OHOS build.
COMPONENTS='utils_native zlib'

if [[ ! -d "$ARK_THIRD_PARTY_DIR" ]] ; then
    echo "Third-party dependencies are not found in $ARK_THIRD_PARTY_DIR, exiting"
    exit 1
fi

pushd "$ARK_THIRD_PARTY_DIR"
    for component in $COMPONENTS; do
        find . -regex ".*/$component.*/BUILD.gn" -exec rm -rf {} \;
    done
popd

exit 0

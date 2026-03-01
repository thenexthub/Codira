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

# The script updates the current fork to the master(you need to create a upstream to the original repository: 
#     git remote add upstream LINK_TO_ORIGINAL_REPO).
# !NOTE the scripts must be run from arkcompiler_runtime_core repo

set -euo pipefail

# get current branch
BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "$BRANCH"

function repeats() {
    local command=$1
    for ((i=1; i < 6; i++)); do
        echo "attempt $i to execute the command \"$command\""
        $command && return 0
    done
    # checkout to current branch
    if [[ "${BRANCH}" != "master" ]]; then
        git checkout "$BRANCH"
        echo "the command \"$command\" failed, checkout to the source branch \"$BRANCH\""
    fi
    return 1
}

git checkout master

# update master in the fork
repeats "git fetch upstream"

echo "git fetch upstream succeeded"
git merge upstream/master
echo "git merge upstream/master succeeded"

repeats "git push origin master"

echo "git push origin master succeeded"

# rebase current branch
if [[ "${BRANCH}" = "master" ]]; then
    echo "current branch is master"
else
    git checkout "$BRANCH"
    echo "git checkout $BRANCH succeeded"

    git rebase master
    echo "git rebase master succeeded"
fi

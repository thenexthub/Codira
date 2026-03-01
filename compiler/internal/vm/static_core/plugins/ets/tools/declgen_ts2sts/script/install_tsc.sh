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

# Install typescript from the third_party folder

set -e

# Check if a root directory parameter is provided, default to the current directory if not
ROOT_DIR="${1:-$(pwd)}"

# Clone the repository and checkout the current Git branch
TYPESCRIPT_REPO="https://gitcode.com/openharmony/third_party_typescript.git"  # Replace with your correct Typescript repository URL
CURRENT_BRANCH="OpenHarmony-v5.0.1-Release"

# Define the third_party directory
THIRD_PARTY_DIR="$ROOT_DIR/third_party"
TYPESCRIPT_ROOT_DIR="$THIRD_PARTY_DIR/typescript"  # Set the directory to 'typescript' inside 'third_party'

# Clone the repository if it doesn't exist, or fetch the latest changes
if [ -d "$TYPESCRIPT_ROOT_DIR" ]; then
    echo "Directory $TYPESCRIPT_ROOT_DIR exists, removing..."
    rm -rf "$TYPESCRIPT_ROOT_DIR"
fi
echo "Cloning the TypeScript repository, tag: $CURRENT_BRANCH"
mkdir -p "$THIRD_PARTY_DIR"
git clone --depth 1 --branch "$CURRENT_BRANCH" "$TYPESCRIPT_REPO" "$TYPESCRIPT_ROOT_DIR"

old_work_dir=$(pwd)
echo "-----------------------------start building typescript-----------------------------"
cd "$TYPESCRIPT_ROOT_DIR"

package_name=$(npm pack | grep "ohos-typescript")
echo "$package_name"
tar -xvf "$package_name"
current_dir=$(pwd)
package_path="$current_dir/package"
echo "-----------------------------end building typescript-----------------------------"

cd "$old_work_dir"
target_dir="$ROOT_DIR/node_modules/typescript"

echo "Move $package_path to override $target_dir"

if [ -d "$target_dir" ]; then
    rm -r "$target_dir"
else
    echo "$target_dir does not exist. No need to remove."
fi

mv "$package_path" "$target_dir"
if [ -d "$TYPESCRIPT_ROOT_DIR" ]; then
    echo "Clean $TYPESCRIPT_ROOT_DIR..."
    rm -rf "$TYPESCRIPT_ROOT_DIR"
fi
echo "------------------------------------finished-------------------------------------"

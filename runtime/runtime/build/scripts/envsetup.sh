#!/bin/bash

# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

# This script needs to be placed in the output directory of Runtime.
# ** NOTE: Please execute envsetup.sh in Codira Compiler before execute this script.
#          Please use `source' command to execute this script. **

# Get the absolute path of the current folder
shell_path=$(readlink -f /proc/$$/exe)
shell_name=${shell_path##*/}

# Get the absolute path of this script according to different shells.
case "${shell_name}" in
    "zsh")
        script_dir=$(cd "$(dirname "$(readlink -f "${(%):-%N}")")"; pwd)
        ;;
    "sh" | "bash")
        script_dir=$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"; pwd)
        ;;
    *)
        echo "[ERROR] Unsupported shell: ${shell_name}, please switch to bash, sh or zsh."
        return 1
        ;;
esac

# Create soft link
if [ -z "$CODIRA_HOME" ]; then
    echo "Please execute envsetup.sh in Codira Compiler first"
elif [ -e "${CODIRA_HOME}/runtime" ]; then
    rm -r ${CODIRA_HOME}/runtime
    ln -s $script_dir ${CODIRA_HOME}/runtime
else
    ln -s $script_dir ${CODIRA_HOME}/runtime

fi
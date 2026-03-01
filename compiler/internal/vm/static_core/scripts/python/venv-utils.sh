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

function get_my_home()
{
    local user_name=$1
    local my_home
    my_home=$(grep "^${user_name}:" /etc/passwd | cut -d: -f6)
    if [[ ! -e "${my_home}" ]]; then
        my_home=/home/${user_name}
    fi
    echo "${my_home}"
}

DOT_VENV_PANDA=.venv-panda
MY_USERNAME=${SUDO_USER}
if [[ -z "${VENV_DIR}" && -n "${MY_USERNAME}" ]]; then
    my_home=$( get_my_home "${MY_USERNAME}" )
    VENV_DIR=${my_home}/${DOT_VENV_PANDA}
elif [[ -z "${VENV_DIR}" && -z "${MY_USERNAME}" ]]; then
    if [[ -n "${USER}" ]]; then
        my_home=$( get_my_home "${USER}" )
        VENV_DIR=${my_home}/${DOT_VENV_PANDA}
    else
        VENV_DIR=/root/${DOT_VENV_PANDA}
    fi
fi

function activate_venv()
{
    if [[ -d "${VENV_DIR}" ]]; then
        source "${VENV_DIR}/bin/activate"
        echo "${VENV_DIR} activated"
    fi
}

function deactivate_venv()
{
    if [[ -d "${VENV_DIR}" && -n "${VIRTUAL_ENV}" ]]; then
        deactivate
    fi
}

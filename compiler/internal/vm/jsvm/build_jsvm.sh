#!/bin/bash
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

declare SCRIPT_PATCH
declare SYSROOT
declare PREFIX
declare TARGET_CPU
declare TARGET_GEN_DIR
declare TARGET_PLATFORM
declare JS_ENGINE_URL
declare DEPENDENCY_TAG

export SCRIPT_PATCH=$(dirname $(readlink -f "$0"))

source ${SCRIPT_PATCH}/build_jsvm_inter.sh

options="$(getopt -o h "help,sysroot:,jsvm_path:,prefix:,target_cpu:,base_path:" -- "$@")" || usage

eval set -- "$options"

usage() {
    echo "Tool $(basename "$0") Usage"
    echo "Options:"
    echo "-h|--help"
    echo "--sysroot <path>            Sysroot path."
    echo "--prefix <perfix>           Cross-compiler prefix."
    echo "--target_cpu <arm/arm64>    Cross-compile CPU types."
    echo "--target_gen_dir <output path>"
    echo "          if target_gen_dir not set, will install only to the default path."
    exit 0
}

die() {
    echo $@
    exit 0
}

do_man_process() {
    do_opt_process $@
    if [[ "${IS_ASAN}" = "true" && "${USE_HWASEN}" = "true" ]]; then
        do_env 1
        do_fetch        > ${out_dir}/log.do_fetch_asan_unstripped
        do_patch        > ${out_dir}/log.do_patch_asan_unstripped
        do_configure    > ${out_dir}/log.do_configure_asan_unstripped
        do_compile      > ${out_dir}/log.do_compile_asan_unstripped
        do_install_asan > ${out_dir}/log.do_install_asan_unstripped
    else
        do_env 0
        do_fetch     > ${out_dir}/log.do_fetch
        do_patch     > ${out_dir}/log.do_patch
        do_configure > ${out_dir}/log.do_configure
        do_compile   > ${out_dir}/log.do_compile
        do_install   > ${out_dir}/log.do_install
        do_strip

        do_env 1
        do_fetch     > ${out_dir}/log.do_fetch_unstripped
        do_patch     > ${out_dir}/log.do_patch_unstripped
        do_configure > ${out_dir}/log.do_configure_unstripped
        do_compile   > ${out_dir}/log.do_compile_unstripped
        do_unstripped_copy
    fi
}

do_opt_process() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
        -h|--help)
            usage
            ;;
        --sysroot)
            export SYSROOT=$2
            shift
            ;;
        --jsvm_path)
            export JSVM_PATH=$2
            shift
            ;;
        --prefix)
            export PREFIX=$2
            export JS_ENGINE_URL=${PREFIX=}/../..
            shift
            ;;
        --target_cpu)
            export TARGET_CPU=$2
            shift
            ;;
        --target_platform)
            export TARGET_PLATFORM=$2
            shift
            ;;
        --target_gen_dir)
            export TARGET_GEN_DIR=$2
            shift
            ;;
        --target_clang_coverage)
            export TARGET_CLANG_COVERAGE=$2
            shift
            ;;
        --is_asan)
            export IS_ASAN=$2
            shift
            ;;
        --use_hwasan)
            export USE_HWASEN=$2
            shift
            ;;
        --cmake_path)
            export CMAKE_PATH=$2
            shift
            ;;
        --target_out_dir)
            export LIB_ROOT_DIR=$2
            shift
            ;;
        --dependency_tag)
            export DEPENDENCY_TAG=$2
            shift
            ;;
        -I*)
            export INCLUDE_DIRS="${INCLUDE_DIRS} -I${LIB_ROOT_DIR}/${1#-I}"
            ;;
        *)
            ;;
        esac
        shift
    done
}

do_man_process $@

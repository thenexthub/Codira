#!/bin/bash
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -x
set -e
set -o pipefail

OHOS_DIR=""
SANITIZERS=false
COVERAGE=false

print_usage() {
    set +x
    echo "Usage: self-check.sh [options] [mode] --dir=<ohos_dir>    "
    echo "                                                          "
    echo "      <ohos_dir> -- path to OHOS root                     "
    echo "Mode:                                                     "
    echo "  -s, --sanitize  build with sanitizers                   "
    echo "  -c, --coverage  collect code coverage                   "
    echo "                                                          "
    echo "Options:                                                  "
    echo "  -h, --help      print help text                         "
    set -x
}

for i in "$@"; do
    case "$i" in
    -c | --coverage)
        COVERAGE=true
        shift
        ;;
    -s | --sanitize)
        SANITIZERS=true
        shift
        ;;
    --dir=*)
        OHOS_DIR="${i#*=}"
        OHOS_DIR="${OHOS_DIR/#\~/$HOME}"
        shift
        ;;
    -h | --help)
        print_usage
        exit 1
        ;;
    *)
        echo "Unknown option \"$i\""
        print_usage
        exit 1
        ;;
    esac
done

build_standalone() {
    set -e
    rm -rf out/$TARGET

    ./ark.py $TARGET abckit_packages --gn-args="is_standard_system=true abckit_with_sanitizers=$SANITIZERS enable_notice_collection=false abckit_with_coverage=$COVERAGE ohos_components_checktype=3 abckit_enable=true abckit_enable_tests=true enable_cmc_gc=false"

    set +e
}

run_check_clang_format() {
    set -e
    ninja abckit_check_clang_format
    set +e
}

run_check_clang_tidy() {
    set -e
    ninja abckit_check_clang_tidy
    set +e
}

run_check_documentation() {
    set -e
    ninja abckit_documentation
    set +e
}

build_and_run_tests() {
    set -e
    ninja abckit_gtests

    LSAN_OPTIONS=""
    if [ "$SANITIZERS" = "true" ]; then
        LSAN_OPTIONS="suppressions=$OHOS_DIR/arkcompiler/runtime_core/libabckit/tests/sanitizers/ignored_leaks.supp"

        if [ -f /etc/ld.so.preload ]; then
            echo "/etc/ld.so.preload detected! Trying to manually specify LD_PRELOAD"
            export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.6
        fi
    fi

    if [ "$COVERAGE" = "true" ]; then
        export LLVM_PROFILE_FILE="abckit%m.profraw"
        export LIBABCKIT_DEBUG_MODE="1"
    fi

    LD_LIBRARY_PATH=./arkcompiler/runtime_core/:./arkcompiler/ets_runtime/:./thirdparty/icu/:./thirdparty/zlib/:./thirdparty/libuv/:./arkui/napi/:./thirdparty/bounds_checking_function/:./arkui/napi/ \
        LSAN_OPTIONS="$LSAN_OPTIONS" \
        ASAN_OPTIONS=verify_asan_link_order=0 \
        ./tests/unittest/arkcompiler/runtime_core/libabckit/abckit_gtests

    if [ "$SANITIZERS" = "false" ]; then
        ninja abckit_stress_tests_package

        if [ "$TARGET" = "x64.debug" ]; then
            ../../arkcompiler/runtime_core/libabckit/tests/stress/runner.py --build-dir $(realpath .)
        else
            ninja ark_js_vm
            ../../arkcompiler/runtime_core/libabckit/tests/stress/stress_js_full.py --build-dir $(realpath .)
            ../../arkcompiler/runtime_core/libabckit/tests/stress/stress_hermes_full.py --build-dir $(realpath .)
        fi
    fi

    if [ "$COVERAGE" = "true" ]; then
        ../../prebuilts/clang/ohos/linux-x86_64/llvm/bin/llvm-profdata merge -sparse abckit*.profraw -o abckit.profdata

        ../../prebuilts/clang/ohos/linux-x86_64/llvm/bin/llvm-cov show \
            --instr-profile=./abckit.profdata \
            --summary-only --output-dir=abckit_coverage \
            ./arkcompiler/runtime_core/libabckit.so

        ../../prebuilts/clang/ohos/linux-x86_64/llvm/bin/llvm-cov report \
            --ignore-filename-regex='(.*third_party/.*|.*/runtime_core/static_core/.*|.*/runtime_core/libpandabase/.*|.*/arkcompiler/ets_frontend/|.*/runtime_core/abc2program/.*|.*/runtime_core/libpandafile/.*|.*/runtime_core/assembler/.*|.*/runtime_core/platforms/.*|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/adapter_static/generated/get_intrinsic_id_static.inc|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/codegen_intrinsics_static.cpp|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/generate_ecma.inl|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/insn_info.h|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/insn_selection_static.cpp)' \
            --instr-profile=./abckit.profdata ./arkcompiler/runtime_core/libabckit.so |
            sed 's|\([^ ]\) \([^ ]\)|\1_\2|g' |
            tr -s ' ' |
            grep -v '\------------' |
            grep -v 'Files_which_contain_no_functions:' |
            sed 's| |,|g' >abckit_coverage.csv

        ../../prebuilts/clang/ohos/linux-x86_64/llvm/bin/llvm-cov report \
            --ignore-filename-regex='(.*third_party/.*|.*/runtime_core/libabckit/src/adapter_static/.*|.*/runtime_core/static_core/.*|.*/runtime_core/abc2program/.*|.*/runtime_core/libpandabase/.*|.*/arkcompiler/ets_frontend/|.*/runtime_core/libpandafile/.*|.*/runtime_core/assembler/.*|.*/runtime_core/platforms/.*|.*/runtime_core/libabckit/src/codegen/generated/insn_selection_static.cpp|.*/runtime_core/libabckit/src/codegen/codegen_static.cpp|.*/runtime_core/libabckit/src/codegen/generated/codegen_intrinsics_static.cpp|.*/runtime_core/libabckit/src/isa_static_impl.cpp|.*/runtime_core/libabckit/src/codegen/codegen_static.h|.*/runtime_core/libabckit/src/codegen/generated/codegen_visitors_static.inc|.*/runtime_core/libabckit/src/codegen/generated/codegen_call_intrinsics_static.inc|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/adapter_static/generated/get_intrinsic_id_static.inc|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/codegen_intrinsics_static.cpp|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/generate_ecma.inl|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/insn_info.h|.*out/x64.debug/gen/arkcompiler/runtime_core/libabckit/src/codegen/generated/insn_selection_static.cpp)' \
            --instr-profile=./abckit.profdata ./arkcompiler/runtime_core/libabckit.so |
            sed 's|\([^ ]\) \([^ ]\)|\1_\2|g' |
            tr -s ' ' |
            grep -v '\------------' |
            grep -v 'Files_which_contain_no_functions:' |
            sed 's| |,|g' >abckit_dynamic_coverage.csv

        echo "Summary abckit coverage report file: $(realpath abckit_coverage.csv)"
        echo "Summary abckit dynamic coverage report file: $(realpath abckit_dynamic_coverage.csv)"
        echo "Verbose abckit coverage report dir: $(realpath abckit_coverage)"
    fi

    if [ "$SANITIZERS" = "true" ]; then
        rm -f "$TMP"
    fi

    set +e
}

if [ -z "$OHOS_DIR" ]; then
    echo "ERROR: Path to OHOS root was not provided"
    print_usage
    exit 1
fi

TARGET=x64.debug
pushd "$OHOS_DIR" || exit 1
build_standalone
pushd out/$TARGET || exit 1
run_check_documentation
run_check_clang_format
if [ "$COVERAGE" = "false" ] && [ "$SANITIZERS" = "false" ]; then
    run_check_clang_tidy
fi
build_and_run_tests
popd || exit 1

if [ "$COVERAGE" = "false" ] && [ "$SANITIZERS" = "false" ]; then
    TARGET=x64.release
    pushd "$OHOS_DIR" || exit 1
    build_standalone
    pushd out/$TARGET || exit 1
    run_check_documentation
    run_check_clang_format
    run_check_clang_tidy
    build_and_run_tests
    popd || exit 1
fi

popd || exit 1

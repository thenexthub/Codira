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

for ARGUMENT in "$@"; do
case "$ARGUMENT" in
    --binary-dir=*)
    PANDA_BINARY_ROOT="${ARGUMENT#*=}"
    ;;
    --root-dir=*)
    PANDA_ROOT="${ARGUMENT#*=}"
    ;;
    --target-arch=*)
    TARGET_ARCH="${ARGUMENT#*=}"
    ;;
    --paoc-mode=*)
    PAOC_MODE="${ARGUMENT#*=}"
    ;;
    -build-panda)
    BUILD_PANDA=true
    ;;
esac
done

if [ "$BUILD_PANDA" = true ]; then
    cd "$PANDA_BINARY_ROOT"
    make -j
    cd -
fi

TARGET_NAME=NULL
if [ "$TARGET_ARCH" = arm64 ]; then
    TARGET_NAME=ARM64
elif [ "$TARGET_ARCH" = arm ]; then
    TARGET_NAME=ARM32
elif [ "$TARGET_ARCH" = x86_64 ]; then
    TARGET_NAME=X86_64
else
    echo "Unsupported arch: $TARGET_ARCH"
    exit 1
fi

PAOC_MODE_NAME=NULL
if [ "$PAOC_MODE" = aot ]; then
    PAOC_MODE_NAME=AOT
elif [ "$PAOC_MODE" = jit ]; then
    PAOC_MODE_NAME=JIT
elif [ "$PAOC_MODE" = osr ]; then
    PAOC_MODE_NAME=OSR
else
    echo "Unsupported paoc-mode: $PAOC_MODE"
    exit 1
fi

TEMP_DIR=$(pwd)/benchmark_coverage_${PAOC_MODE_NAME}_${TARGET_NAME}
mkdir -p "$TEMP_DIR"
cd "$TEMP_DIR"

PANDASM=$PANDA_BINARY_ROOT/bin/ark_asm
PAOC=$PANDA_BINARY_ROOT/bin/ark_aot
BENCHMARKS=$PANDA_ROOT/tests/benchmarks
SMALL_TESTS=$PANDA_ROOT/tests/cts-assembly

ARKSTDLIB="$PANDA_BINARY_ROOT/pandastdlib/arkstdlib.abc"

MAX_BYTECODE_SIZE=40000

IGNORE_FUNCTIONS=$PANDA_ROOT/compiler/tools/ignore_checked_coverage.txt
LOG_ERROR=$PANDA_BINARY_ROOT/compiler/coverage/log_compiler_error_${PAOC_MODE}_${TARGET_ARCH}.txt
HTML=$PANDA_BINARY_ROOT/compiler/coverage/BenchmarkCoverage_${PAOC_MODE}_${TARGET_ARCH}.html
mkdir -p "${PANDA_BINARY_ROOT}/compiler/coverage"

[ -e "$HTML" ] && rm "$HTML"
[ -e "$LOG_ERROR" ] && rm "$LOG_ERROR"

critical_compiler_failures=()
not_compiled_benchmarks=()

# Checks if a test is in the ignore list:
function check_critical_failure() {
    status=0
    local filtered_ignore_list=$(grep "^IGNORE.*$1" "$IGNORE_FUNCTIONS") || status=$?
    # Check if the ignore list contains lines with current benchmark/CTS-test name:
    if [ "${status}" -ne 0 ]; then
        critical_compiler_failures+=( "$1:$name" )
        return
    fi
    while IFS= read -r line; do
        # Check if methods were specified:
        if (! echo "$line" | grep -q "FUNCS") || (echo "$line" | grep -q "$name"); then
            # Check architecture ignore compound:
            if (! echo "$line" | grep -q "ARM64\|ARM32\|X86_64") || (echo "$line" | grep -q "$TARGET_NAME"); then
                # Check compilation mode compound:
                if (! echo "$line" | grep -q "OSR\|JIT") || (echo "$line" | grep -q "$PAOC_MODE_NAME"); then
                    return
                fi
            fi
        fi
    done <<< "$filtered_ignore_list"
    critical_compiler_failures+=( "$1:$name" )
    return
}

function separate_method_name() {
    if [[ "$1" == *\.* ]]; then
        method_name=$(echo "$1" | rev | cut -f 1 -d '.' | rev)
        class_name=$(echo "$1" | rev | cut -f 2- -d '.' | rev)
    else
        class_name="_GLOBAL"
        method_name=$1
    fi
}

function calculate_benchmark_coverage() {
    local all_functions=0
    local all_compiled=0
    local all_assert=0

    echo "<header><h1>Benchmark coverage statistic</h1></header>" >> "$HTML"

    for benchmark in $(find "$BENCHMARKS"/*\.pa -maxdepth 0 -exec basename -s .pa {} \;); do
        # Сhecking that benchmark has compiled
        if $PANDASM "$BENCHMARKS"/"$benchmark".pa "$benchmark".abc
        then
            local method_names=$(grep "^[.]function" "${BENCHMARKS}/${benchmark}".pa \
                | grep -v "<[^>]*external[^>]*>" | cut -f3 -d' ' | cut -f1 -d'(')
            local functions=0
            local compiled=0
            local assert=0
            local func_status=()

            for name in $method_names; do
                let "functions+=1"
                # Get class and method names
                separate_method_name "$name"
                local tmp_file=$benchmark-$class_name-$method_name.tmp
                rm -f "$tmp_file"
                local compiler_status=0
                $PAOC --paoc-panda-files="$benchmark.abc" --compiler-regex="$class_name::$method_name" \
                        --compiler-ignore-failures=false --paoc-mode="$PAOC_MODE" \
                        --boot-panda-files="$ARKSTDLIB" --load-runtimes="core" \
                        --compiler-max-bytecode-size=$MAX_BYTECODE_SIZE \
                        --compiler-cross-arch="$TARGET_ARCH" --log-level=debug --log-components=compiler > "$tmp_file" 2>&1 || \
                    compiler_status=$?
                if [ "$compiler_status" -eq 0 ]; then
                    let "compiled+=1"
                    func_status+=( "compiled" )
                else
                    if grep -q "RunOptimizations failed!" "$tmp_file" ; then
                        func_status+=( "optimizations" )

                    elif grep -q "IrBuilder failed!" "$tmp_file" ; then
                        func_status+=( "ir_builder" )
                    else
                        let "assert+=1"
                        func_status+=( "assert" )
                    fi
                    # Checking that the benchmark function should be built and not have asserts
                    check_critical_failure "$benchmark"
                    # Information for reproducing the error
                    echo "Benchmark=$benchmark" >> "$LOG_ERROR"
                    echo "Function=$name" >> "$LOG_ERROR"
                    echo "Status=${func_status[-1]}" >> "$LOG_ERROR"
                    echo "Reproduce:" >> "$LOG_ERROR"
                    echo "$PANDASM $BENCHMARKS/$benchmark.pa $benchmark.abc" >> "$LOG_ERROR"
                    echo -n "$PAOC --paoc-panda-files=\"$benchmark.abc\" --paoc-mode=$PAOC_MODE" >> "$LOG_ERROR"
                    echo -n " --compiler-regex=\"$class_name::$method_name\"" >> "$LOG_ERROR"
                    echo -n " --boot-panda-files=\"$ARKSTDLIB\" --load-runtimes=\"core\"" >> "$LOG_ERROR"
                    echo -n " --compiler-ignore-failures=false" >> "$LOG_ERROR"
                    echo -n " --compiler-cross-arch=$TARGET_ARCH" >> "$LOG_ERROR"
                    echo -n " --compiler-max-bytecode-size=$MAX_BYTECODE_SIZE" >> "$LOG_ERROR"
                    echo " --log-level=debug --log-components=compiler" >> "$LOG_ERROR"
                    echo "" >> "$LOG_ERROR"
                fi
                rm -f "$tmp_file"
            done
            rm -f "$benchmark".abc

            # Benchmark status
            if [ $assert -ne 0 ] || [ $compiled -eq 0 ]; then
                local background="#FF250D"
            elif [ $compiled -ne $functions ] ; then
                local background="yellow"
            else
                local background="lime"
            fi
            # Benchmark statistics
            local coverage=$(($compiled * 100 / $functions))
            echo "<p><b><span style=\"background-color:$background\">$benchmark</span></b>
                functions = $functions,
                compiled = $compiled,
                assert = $assert,
                coverage = $coverage%</p>" >> "$HTML"

            # Names benchmark functions
            echo "<table cellpadding=\"5\"><tr>
                <th align=\"center\">functions</th>" >> "$HTML"
            for name in $method_names; do
                echo "<td align=\"center\">$name</td>" >> "$HTML"
            done

            # Statuses of benchmark functions
            echo "<tr><th align=\"center\">status</th>" >> "$HTML"
            for status in "${func_status[@]}"; do
                echo "<td align=\"center\" bgcolor=" >> "$HTML"
                if [ "$status" == "compiled" ]; then
                    echo "\"#d7e7a9\">compiled" >> "$HTML"
                elif [ "$status" == "ir_builder" ]; then
                    echo "\"salmon\">ir_builder" >> "$HTML"
                elif [ "$status" == "optimizations" ]; then
                    echo "\"salmon\">optimizations" >> "$HTML"
                else
                    echo "\"#F5001D\">assert" >> "$HTML"
                fi
            done
            echo "</table>" >> "$HTML"

            all_functions=$((all_functions+$functions))
            all_compiled=$((all_compiled+$compiled))
            all_assert=$((all_assert+$assert))
        else
            not_compiled_benchmarks+=( "$benchmark" )
        fi
    done

    # Benchmark report
    local benchmark_coverage=$(($all_compiled * 100 / $all_functions))
    echo "Benchmark report $TARGET_ARCH:"
    echo "Functions = $all_functions | Compiled = $all_compiled | Assert = $all_assert | Coverage = $benchmark_coverage%"
}

function calculate_assembly_tests_coverage() {
    local all_tests=0
    local success_tests=0

    # CTS-assembly test
    echo "<header><h1>CTS-ASSEMBLY TESTS</h1></header>
        <table cellpadding=\"5\">" >> "$HTML"

    for test in $(ls "$SMALL_TESTS" | cut -f1 -d'.'); do
        # Сhecking that tests has compiled
        if $PANDASM "${SMALL_TESTS}/${test}.pa" "${test}.abc"; then
            local method_names=$(grep "^[.]function" "${SMALL_TESTS}/${test}.pa" | grep -v "<[^>]*external[^>]*>\|<[^>]*noimpl[^>]*>" | cut -f3 -d' ' | cut -f1 -d'(')
            local functions=0
            local compiled=0
            local assert=0

            for name in $method_names;  do
                let "functions+=1"

                local is_cctor=1
                local is_ctor=1
                grep -q "${name}()\s*<.*cctor.*>" "${SMALL_TESTS}/${test}.pa" || local is_cctor=$(($?^1))
                grep -q "${name}(.*)\s*<.*ctor.*>" "${SMALL_TESTS}/${test}.pa" || local is_ctor=$(($?^1))
                # Get class and method names
                separate_method_name "$name"
                local method_name=$method_name
                if [ "$is_cctor" -eq 1 ]; then
                    local method_name=".cctor"
                elif [ "$is_ctor" -eq 1 ]; then
                    local method_name=".ctor"
                fi
                local load_runtimes="core"
                local boot_panda_files=$ARKSTDLIB

                local tmp_file=$test-$class_name-$method_name.tmp
                rm -f "$tmp_file"
                local compiler_status=0
                $PAOC --paoc-panda-files="$test".abc --compiler-regex="$class_name::$method_name" \
                                    --compiler-ignore-failures=false --paoc-mode="$PAOC_MODE" \
                                    --boot-panda-files="$boot_panda_files" \
                                    --load-runtimes="$load_runtimes" \
                                    --compiler-max-bytecode-size="$MAX_BYTECODE_SIZE" \
                                    --compiler-cross-arch="$TARGET_ARCH" --log-level=debug --log-components=compiler > "$tmp_file" 2>&1 || \
                compiler_status=$?
                if [ "$compiler_status" -eq 0 ]; then
                    let "compiled+=1"
                else
                    if grep -q "RunOptimizations failed!" "$tmp_file"; then
                        local test_status="optimizations"
                    elif grep -q "IrBuilder failed!" "$tmp_file"; then
                        local test_status="ir_builder"
                    else
                        let "assert+=1"
                        local test_status="assert"
                    fi
                    # Checking that the benchmark function should be built and not have asserts
                    check_critical_failure "$test"
                    # Information for reproducing the error
                    echo "CTS-assembly test=$test" >> "$LOG_ERROR"
                    echo "Function=$name" >> "$LOG_ERROR"
                    echo "Status=$test_status" >> "$LOG_ERROR"
                    echo "Reproduce:" >> "$LOG_ERROR"
                    echo "$PANDASM $SMALL_TESTS/$test.pa $test.abc" >> "$LOG_ERROR"
                    echo -n "$PAOC --paoc-panda-files=\"$test.abc\" --paoc-mode=$PAOC_MODE" >> "$LOG_ERROR"
                    echo -n " --compiler-regex=\"$class_name::$method_name\"" >> "$LOG_ERROR"
                    echo -n " --compiler-ignore-failures=false" >> "$LOG_ERROR"
                    echo -n " --boot-panda-files=\"$boot_panda_files\"" >> "$LOG_ERROR"
                    echo -n " --load-runtimes=\"$load_runtimes\"" >> "$LOG_ERROR"
                    echo -n " --compiler-cross-arch=$TARGET_ARCH" >> "$LOG_ERROR"
                    echo -n " --compiler-max-bytecode-size=$MAX_BYTECODE_SIZE" >> "$LOG_ERROR"
                    echo " --log-level=debug --log-components=compiler --compiler-log=all" >> "$LOG_ERROR"
                    echo "" >> "$LOG_ERROR"
                fi
                rm -f "$tmp_file"
            done
            rm -f "$test.abc"

            local indent=$(($all_tests % 10))
            if [ $indent -eq 0 ]; then
                echo "<tr>" >> "$HTML"
            fi

            # Test status
            if [ $assert -ne 0 ] || [ $compiled -eq 0 ]; then
                local background="red"
            elif [ $compiled -ne $functions ]; then
                local background="yellow"
            else
                local background="#d7e7a9"
                let "success_tests+=1"
            fi
            echo "<td align=\"center\" bgcolor=$background>$test</td>" >> "$HTML"

            let "all_tests+=1"
        fi
    done
    echo "</table>" >> "$HTML"

    # CTS-assembly tests report
    local coverage=$(($success_tests * 100 / $all_tests))
    echo "<h4>Coverage tests = $coverage%</h4>" >> "$HTML"
    echo "CTS-assembly tests coverage = $coverage%"
}


echo "<!DOCTYPE html>
<html>
    <head>
        <style>table, th, td {border: 1px solid black; border-collapse: collapse;}</style>
        <title>Benchmark coverage statistic</title>
    </head>
    <body>" >> "$HTML"

calculate_benchmark_coverage

calculate_assembly_tests_coverage

rm -r "$TEMP_DIR"

if [ ${#critical_compiler_failures[@]} -ne 0 ]; then
    echo "The following functions must be compiled:"
    for failure in ${critical_compiler_failures[@]}; do
        echo "    $failure"
    done
    echo "If you are sure that your actions are correct:
    * add the ignore flag for the specified functions in the $IGNORE_FUNCTIONS
    * create an issue the error description with assignee @pishin"
    echo "To reproduce the error, see: $LOG_ERROR"
    exit 1
fi

echo "<h3>Assembler failed to build the following benchmarks:</h3>" >> "$HTML"
for benchmark in "${not_compiled_benchmarks[@]}"; do
    echo "<p>$benchmark</p>" >> "$HTML"
done


echo  "<h3>Benchmark status</h3>
        <table cellpadding=\"5\">
            <tr>
                <th align=\"left\">Compiled all function</th>
                <td bgcolor=\"lime\"></td>
            </tr>
            <tr>
                <th align=\"left\">Compiled some function and doesn't have assert</th>
                <td bgcolor=\"yellow\"></td>
            </tr>
            <tr>
                <th align=\"left\">Has assert or all functions are not compiled</th>
                <td bgcolor=\"#FF250D\"></td>
            </tr>
        </table>
        <h3>Function status</h3>
        <table cellpadding=\"5\">
            <tr>
                <th align=\"left\">Successfully compiled</th>
                <td align=\"center\" bgcolor=\"#d7e7a9\">compiled</td>
            </tr>
            <tr>
                <th align=\"left\">Failed to build ir</th>
                <td align=\"center\" bgcolor=\"salmon\">ir_builder</td>
            </tr>
            <tr>
                <th align=\"left\">Optimization failed</th>
                <td align=\"center\" bgcolor=\"salmon\">optimizations</td>
            </tr>
            <tr>
                <th align=\"left\">ASSERT thrown</th>
                <td align=\"center\" bgcolor=\"#F5001D\">assert</td>
            </tr>
        </table>
    </body>
</html>" >> "$HTML"

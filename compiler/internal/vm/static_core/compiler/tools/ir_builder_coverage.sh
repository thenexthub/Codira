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

ISA=$PANDA_BINARY_ROOT/isa/isa.yaml
IR_BUILDER_TESTS=$PANDA_ROOT/compiler/tests/ir_builder_test.cpp
UNIT_TESTS=$PANDA_BINARY_ROOT/bin-gtests/compiler_unit_tests
HTML=$PANDA_BINARY_ROOT/compiler/IrBuilderCoverage.html

[ -e "$HTML" ] && rm "$HTML"

all_tests=0
all_compiled=0

# Get signatures of all instructions
all_insts=$(grep "sig:" "$ISA" | cut -f2 -d':' | cut -f2 -d' ')
# Ignore repeated instructions
pbc_instructions=$(echo "$all_insts" | awk '{delete seen; c=0; for (i=1;i<=NF;i++) \
    if (!seen[$i]++) printf "%s%s", (++c>1?OFS:""), $i; print ""}')

function get_test_name() {
    # For example:
    #    mov.64 => Mov64
    #    fadd2.64 => Fadd2_64
    #    call.short => CallShort
    test_name=$(echo ."${inst_name}" | sed 's/\.\([a-z]\)/\U\1/g' | sed 's/\([0-9]\)\.\([0-9]\)/\1_\2/g' | sed 's/\.//g')
}

function find_test() {
    coverage=${coverage:-0}
    inst_status=${inst_status:-1}
    if $(grep "TEST_F(IrBuilderTest, $1)" "$IR_BUILDER_TESTS" 1>/dev/null); then
        coverage=1
        $UNIT_TESTS --gtest_filter=IrBuilderTest."$1" 1>/dev/null
        if [ $? != 0 ]; then
            inst_status=0
        fi
    elif $(grep "TEST_F(IrBuilderTest, DISABLED_$1)" "$IR_BUILDER_TESTS" 1>/dev/null); then
        inst_status=0
    fi
}

function calculate_ir_builder_coverage() {
    for inst_name in $pbc_instructions; do
        get_test_name
        find_test "${test_name}"
        find_test "${test_name}Int"
        find_test "${test_name}Real"

        indent=$(($all_tests % 5))
        if [ $indent -eq 0 ] ; then
            echo "<tr>" >> "$HTML"
        fi

        if [ "$inst_status" == 1 ] ; then
            if [ "$coverage" == 1 ] ; then
                let "all_compiled+=1"
                background="#d7e7a9"
            else
                background="gray"
            fi
        else
            background="red"
        fi
        echo "<td align=\"center\" bgcolor=$background>$inst_name</td>" >> "$HTML"
        let "all_tests+=1"
    done
}

echo "<!DOCTYPE html>
<html>
    <head>
        <style>table, th, td {border: 1px solid black; border-collapse: collapse;}</style>
        <title>Benchmark coverage statistic</title>
    </head>
    <body>" >> "$HTML"

echo "<header><h1>IR BUILDER TESTS</h1></header>
        <table cellpadding=\"5\">" >> "$HTML"

calculate_ir_builder_coverage

echo "</table>" >> "$HTML"

# Report
ir_builder_coverage=$(($all_compiled * 100 / $all_tests))
echo "<h3>Instructions coverage = $ir_builder_coverage%</h3>" >> "$HTML"
echo "Ir_builder coverage: $ir_builder_coverage%"

echo  "<h2>Legend</h2>
        <table cellpadding=\"5\">
            <tr><td align=\"left\">All tests for the instructions are passed</td><td bgcolor=\"#d7e7a9\"></td></tr>
            <tr><td align=\"left\">Some tests for instructions failed or disabled</td><td bgcolor=\"red\"></td></tr>
            <tr><td align=\"left\">The instruction is not covered by tests</td><td bgcolor=\"gray\"></td></tr>
        </table>" >> "$HTML"

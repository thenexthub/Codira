/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       AOT IR Builder, check StringBuilder append calls merging
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*append[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_0_if1_else1_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_1_if1_else1_1_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_2_if1_else1_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_1_if2_else1_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_0_if2_else2_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging  (--gc-type=stw)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part2.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_0_if1_else1_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_1_if1_else1_1_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_2_if1_else1_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_1_if2_else1_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part2.ETSGLOBAL::_0_if2_else2_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging (--gc-type=g1-gc)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--gc-type=g1-gc --no-async-jit=false --compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part2.ETSGLOBAL::main"

function _0_if1_else1_2_append4(cond: boolean): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    if (cond)
        sb.append("0");
    else
        sb.append("1");
    sb.append("2");
    sb.append("3");

    return sb.toString();           // applied, 1+1+2
}

function _1_if1_else1_1_append4(cond: boolean): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    if (cond)
        sb.append("1");
    else
        sb.append("2");
    sb.append("3");

    return sb.toString();           // not applied, 1+1+1+1
}

function _2_if1_else1_0_append4(cond: boolean): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    if (cond)
        sb.append("2");
    else
        sb.append("3");

    return sb.toString();           // applied, 2+1+1
}

function _1_if2_else1_0_append4(cond: boolean): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    if (cond) {
        sb.append("1");
        sb.append("2");
    }
    else
        sb.append("3");

    return sb.toString();           // applied, 1+2+1
}

function _0_if2_else2_0_append4(cond: boolean): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    if (cond) {
        sb.append("0");
        sb.append("1");
    }
    else {
        sb.append("2");
        sb.append("3");
    }

    return sb.toString();           // applied, 2+2
}

function main() : int {
    arktest.assertEQ(_0_if1_else1_2_append4(true ), "023", 'Wrong result at _0_if1_else1_2_append4(true )');
    arktest.assertEQ(_0_if1_else1_2_append4(false), "123", 'Wrong result at _0_if1_else1_2_append4(false)');
    arktest.assertEQ(_1_if1_else1_1_append4(true ), "013", 'Wrong result at _1_if1_else1_1_append4(true )');
    arktest.assertEQ(_1_if1_else1_1_append4(false), "023", 'Wrong result at _1_if1_else1_1_append4(false)');
    arktest.assertEQ(_2_if1_else1_0_append4(true ), "012", 'Wrong result at _2_if1_else1_0_append4(true )');
    arktest.assertEQ(_2_if1_else1_0_append4(false), "013", 'Wrong result at _2_if1_else1_0_append4(false)');
    arktest.assertEQ(_1_if2_else1_0_append4(true ), "012", 'Wrong result at _1_if2_else1_0_append4(true )');
    arktest.assertEQ(_1_if2_else1_0_append4(false), "03" , 'Wrong result at _1_if2_else1_0_append4(false)');
    arktest.assertEQ(_0_if2_else2_0_append4(true ), "01" , 'Wrong result at _0_if2_else2_0_append4(true )');
    arktest.assertEQ(_0_if2_else2_0_append4(false), "23" , 'Wrong result at _0_if2_else2_0_append4(false)');

    return 0
}

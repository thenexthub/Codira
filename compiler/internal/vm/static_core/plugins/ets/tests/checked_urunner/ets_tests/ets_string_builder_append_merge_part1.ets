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
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_1_toString_7_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_2_toString_6_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_3_toString_5_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_4_toString_4_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_5_toString_3_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_6_toString_2_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_7_toString_1_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_1_append_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_1_phi_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging  (--gc-type=stw)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part1.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_1_toString_7_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_2_toString_6_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_3_toString_5_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_4_toString_4_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_5_toString_3_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_6_toString_2_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_7_toString_1_append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_1_append_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//!
//! METHOD        "ets_string_builder_append_merge_part1.ETSGLOBAL::_1_phi_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging (--gc-type=g1-gc)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--gc-type=g1-gc --no-async-jit=false --compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part1.ETSGLOBAL::main"

function _1_toString_7_append8(): string {
    let sb = new StringBuilder();

    sb.append("0");
    sb.toString();
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 1+4+3
}

function _0_toString_8_append8(): string {
    let sb = new StringBuilder();

    sb.toString();
    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 4+4
}

function _2_toString_6_append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.toString();
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 2+4+2
}

function _3_toString_5_append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.toString();
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 3+4+1
}

function _4_toString_4_append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.toString();
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 4+4
}

function _5_toString_3_append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.toString();
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 4+1+3
}

function _6_toString_2_append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.toString();
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 4+2+2
}

function _7_toString_1_append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.toString();
    sb.append("7");

    return sb.toString();           // applied, 4+3+1
}

function append(sb: StringBuilder, s: string): void {
    sb.append(s);
}

function _1_append_2_append4(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    append(sb, "1");
    sb.append("2");
    sb.append("3");

    return sb.toString();           // applied, 1+append+2
}

function _1_phi_2_append4(cond: boolean): String {
    let sb0 = new StringBuilder();
    let sb1 = new StringBuilder();

    let sb = cond ? sb1 : sb0;

    sb0.append("0");
    sb.append("1");
    sb0.append("2");
    sb0.append("3");

    return sb0.toString();                  // not applied, sb is Phi instruction
}

function main() : int {
    arktest.assertEQ(_0_toString_8_append8(), "01234567", 'Wrong result at _0_toString_8_append8()');
    arktest.assertEQ(_1_toString_7_append8(), "01234567", 'Wrong result at _1_toString_7_append8()');
    arktest.assertEQ(_2_toString_6_append8(), "01234567", 'Wrong result at _2_toString_6_append8()');
    arktest.assertEQ(_3_toString_5_append8(), "01234567", 'Wrong result at _3_toString_5_append8()');
    arktest.assertEQ(_4_toString_4_append8(), "01234567", 'Wrong result at _4_toString_4_append8()');
    arktest.assertEQ(_5_toString_3_append8(), "01234567", 'Wrong result at _5_toString_3_append8()');
    arktest.assertEQ(_6_toString_2_append8(), "01234567", 'Wrong result at _6_toString_2_append8()');
    arktest.assertEQ(_7_toString_1_append8(), "01234567", 'Wrong result at _7_toString_1_append8()');

    arktest.assertEQ(_1_append_2_append4(), "0123", 'Wrong result at _1_append_2_append4');
    arktest.assertEQ(_1_phi_2_append4(false), "0123", 'Wrong result at _1_phi_2_append4(false)');
    arktest.assertEQ(_1_phi_2_append4(true), "023", 'Wrong result at _1_phi_2_append4(true)');

    return 0
}

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
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_0_int_3_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_1_int_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_2_int_1_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_3_int_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_0_toString_5_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_1_toString_4_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_2_toString_3_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_3_toString_2_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_4_toString_1_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_5_toString_0_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging  (--gc-type=stw)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part3.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_0_int_3_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_1_int_2_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_2_int_1_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_3_int_0_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_0_toString_5_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_1_toString_4_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_2_toString_3_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_3_toString_2_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_4_toString_1_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::_5_toString_0_append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging (--gc-type=g1-gc)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--gc-type=g1-gc --no-async-jit=false --compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part3.ETSGLOBAL::main"

function _0_int_3_append4(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append(0);
    sb.append("1");
    sb.append("2");
    sb.append("3");

    return sb.toString();           // applied, int+3
}

function _1_int_2_append4(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append(1);
    sb.append("2");
    sb.append("3");

    return sb.toString();           // applied, 1+int+2
}

function _2_int_1_append4(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append(2);
    sb.append("3");

    return sb.toString();           // applied, 2+int+1
}

function _3_int_0_append4(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append(3);

    return sb.toString();           // applied, 3+int
}

function _0_toString_5_append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append(sb.toString());
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");

    return sb.toString();           // applied, 4+2
}

function _1_toString_4_append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append(sb.toString());
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");

    return sb.toString();           // applied, 1+4+1
}

function _2_toString_3_append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append(sb.toString());
    sb.append("3");
    sb.append("4");
    sb.append("5");

    return sb.toString();           // applied, 2+4
}

function _3_toString_2_append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append(sb.toString());
    sb.append("4");
    sb.append("5");

    return sb.toString();           // applied, 3+3
}

function _4_toString_1_append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append(sb.toString());
    sb.append("5");

    return sb.toString();           // applied, 4+2
}

function _5_toString_0_append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append(sb.toString());

    return sb.toString();           // applied, 4+1+1
}

function main() : int {
    arktest.assertEQ(_0_int_3_append4(), "0123", 'Wrong result at _0_int_3_append4');
    arktest.assertEQ(_1_int_2_append4(), "0123", 'Wrong result at _1_int_2_append4');
    arktest.assertEQ(_2_int_1_append4(), "0123", 'Wrong result at _2_int_1_append4');
    arktest.assertEQ(_3_int_0_append4(), "0123", 'Wrong result at _3_int_0_append4');

    arktest.assertEQ(_0_toString_5_append6(), "12345", 'Wrong result at _0_toString_5_append6');
    arktest.assertEQ(_1_toString_4_append6(), "002345", 'Wrong result at _1_toString_4_append6');
    arktest.assertEQ(_2_toString_3_append6(), "0101345", 'Wrong result at _2_toString_3_append6');
    arktest.assertEQ(_3_toString_2_append6(), "01201245", 'Wrong result at _3_toString_2_append6');
    arktest.assertEQ(_4_toString_1_append6(), "012301235", 'Wrong result at _4_toString_1_append6');
    arktest.assertEQ(_5_toString_0_append6(), "0123401234", 'Wrong result at _5_toString_0_append6');

    return 0
}

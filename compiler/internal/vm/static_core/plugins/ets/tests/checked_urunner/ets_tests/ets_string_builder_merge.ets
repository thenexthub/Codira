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
//! RUN_FRONTEND            options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER       AOT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*chain_concat[0-9]*' --compiler-inlining=false"
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",7
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! INST_COUNT    "Intrinsic.StdCoreSbToString",7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",9
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! INST_COUNT    "Intrinsic.StdCoreSbToString",9
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",14
//! INST_COUNT    "Intrinsic.StdCoreSbToString",14
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",12
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",37
//! INST_COUNT    "Intrinsic.StdCoreSbAppendBool",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendShort",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendLong",2
//! INST_COUNT    "std.core.StringBuilder::append",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",22
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_append_chain_concat7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_constructor_chain_concat8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat10"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat11"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",3
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat12"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat13"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partially_supported_chain_concat14"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat15"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partially_supported_chain_concat16"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat17"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat18"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! RUN           entry: "ets_string_builder_merge.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*chain_concat[0-9]+.*' --compiler-inlining=false", entry: "ets_string_builder_merge.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",7
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! INST_COUNT    "Intrinsic.StdCoreSbToString",7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",9
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! INST_COUNT    "Intrinsic.StdCoreSbToString",9
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",14
//! INST_COUNT    "Intrinsic.StdCoreSbToString",14
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",12
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",37
//! INST_COUNT    "Intrinsic.StdCoreSbAppendBool",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendShort",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendLong",2
//! INST_COUNT    "std.core.StringBuilder::append",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",22
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_append_chain_concat7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_constructor_chain_concat8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat10"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat11"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",3
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat12"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat13"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partially_supported_chain_concat14"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat15"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partially_supported_chain_concat16"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat17"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat18"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! INST_COUNT    "String::concat",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1

// Each '+=' operation on strings in tests below is translated into StringBuilder + 2 append calls by frontend

function chain_concat0(a: String, n: int, b: String, m: int): String {
    let str: String = "";
    str += "_";
    for (let i = 0; i < n; ++i)
        str += a;
    str += "_";
    for (let i = 0; i < m; ++i)
        str += b;
    str += "_";
    return str;                                 // applied, 5 SBs merged into 1
}

function chain_concat1(a: String, n: int, b: String, m: int, c: String, l: int): String {
    let str: String = "";
    str += "_";
    for (let i = 0; i < n; ++i)
        str += a;
    str += "_";
    for (let i = 0; i < m; ++i)
        str += b;
    str += "_";
    for (let i = 0; i < l; ++i)
        str += c;
    str += "_";
    return str;                                 // applied, 7 SBs merged into 1
}

function chain_concat2(a: String, n: int, b: String, m: int, c: String, l: int, d: String, k: int): String {
    let str: String = "";
    str += "_";
    for (let i = 0; i < n; ++i)
        str += a;
    str += "_";
    for (let i = 0; i < m; ++i)
        str += b;
    str += "_";
    for (let i = 0; i < l; ++i)
        str += c;
    str += "_";
    for (let i = 0; i < k; ++i)
        str += d;
    str += "_";
    return str;                                 // applied, 9 SBs merged into 1
}

function chain_concat3(a: String, n: int, b: String, m: int, c: String, l: int, d: String, k: int): String {
    let str0: String = "";                      // two SBs chains '0' and '1'
    let str1: String = "";                      // for each string accumulator variable
    str0 += "_";                                //  0
    str1 += "_";                                //  |     1
    for (let i = 0; i < n; ++i) {               //  |     |
        str0 += a;                              //  0     |
        str1 += a;                              //  |     1
    }                                           //  |     |
    str0 += "_";                                //  0     |
    str1 += "_";                                //  |     1
    for (let i = 0; i < m; ++i) {               //  |    /
        str0 += b;                              //  0   /
        str1 += b;                              //  |  1
    }                                           //  | /
                                                //  |/
    str0 += str1;                               //  0 <- 'chain 1' connects to 'chain 0' here
    str0 += "_";                                //  0
                                                //  |
    for (let i = 0; i < l; ++i)                 //  |
        str0 += c;                              //  0
    str0 += "_";                                //  0
    for (let i = 0; i < k; ++i)                 //  |
        str0 += d;                              //  0
    str0 += "_";                                //  0
    return str0;    // applied, 14 SBs (10 from 'chain 0', and 4 from 'chain 1') merged into 2 SBs
}

// Set of SB based tests

const empty = "";
const space = " ";

function supported_chain_concat5(): String {
    let sb0 = new StringBuilder();
    sb0.append(0)
       .append("1")
       .append(c'2');

    let sb1 = new StringBuilder();
    sb1.append(sb0.toString())
       .append(space)
       .append(0)
       .append("1")
       .append(c'2');

    return sb1.toString();                  // applied
}

function supported_chain_concat6(): String {
    let sb0 = new StringBuilder();
    sb0.append(space)
       .append("abcΣΨΩ0123456789")
       .append(space)
       .append(true)
       .append(space)
       .append(false)
       .append(space)
       .append(c'A' as char)
       .append(space)
       .append(57 as byte)
       .append(space)
       .append(127 as byte)
       .append(space)
       .append(32767 as short)
       .append(space)
       .append(128934675 as int)
       .append(space)
       .append(701234987654321 as long)
       .append(space)
       .append(empty)
       .append("string")
       .append(space)
       .append(new Object())
       .append(space)
       .append(1.0f as Float)
       .append(space)
       .append(1.0 as Double)
       .append(space)
       .append(1.0 as Number)
       .append(space);

    let sb1 = new StringBuilder();
    sb1.append(sb0.toString())
       .append(space)
       .append("abcΣΨΩ0123456789")
       .append(space)
       .append(true)
       .append(space)
       .append(false)
       .append(space)
       .append(c'A' as char)
       .append(space)
       .append(57 as byte)
       .append(space)
       .append(127 as byte)
       .append(space)
       .append(32767 as short)
       .append(space)
       .append(128934675 as int)
       .append(space)
       .append(701234987654321 as long)
       .append(space)
       .append(empty)
       .append("string")
       .append(space)
       .append(new Object())
       .append(space)
       .append(1.0f as Float)
       .append(space)
       .append(1.0 as Double)
       .append(space)
       .append(1.0 as Number)
       .append(space);

    return sb1.toString();                  // applied
}

function unsupported_append_chain_concat7(): String {
    let sb0 = new StringBuilder();
    sb0.append(0);

    let sb1 = new StringBuilder();
    sb1.append(0)
       .append(space)
       .append(sb0.toString());

    return sb1.toString();                  // not applied, sb0.toString() must be appended first
}

function unsupported_constructor_chain_concat8(): String {
    let sb0 = new StringBuilder();
    sb0.append(0);

    let sb1 = new StringBuilder("0");
    sb1.append(sb0.toString());

    return sb1.toString();                  // not applied, sb1 must be constructed via default constructor
}

function unsupported_usage_chain_concat9(): String {
    let sb0 = new StringBuilder();
    sb0.append(0);

    let sb1 = new StringBuilder();
    sb1.append(sb0.toString());

    sb0.append(space);

    return sb1.toString();                  // not applied, sb0 must not be used after sb0.toString() appended
}

function unsupported_chain_concat10(cond: boolean): String {
    let sb0 = new StringBuilder();
    sb0.append(0)
       .append("1")
       .append(c'2');

    let sb1 = new StringBuilder();
    if (cond)
        sb1.append(sb0.toString());
    else
        sb1.append("210");

    sb1.append(space)
       .append(0)
       .append("1")
       .append(c'2');

    return sb1.toString();                  // not applied, sb1 constructor and sb1.append(sb0.toString()) are not in the same BB
}

function unsupported_chain_concat11(cond: boolean): String {
    let sb0 = new StringBuilder("0");
    let sb1 = new StringBuilder("1");

    let sb = cond ? sb1 : sb0;

    let sb2 = new StringBuilder();
    sb2.append(sb.toString())
       .append(space)
       .append(2);

    return sb2.toString();                  // not applied, sb is Phi instruction
}

function __noinline__consume(obj: Object): void {}

function unsupported_chain_concat12(cond: boolean): String {
    let sb0 = new StringBuilder("0");
    let sb1 = new StringBuilder();
    let sb = cond ? sb1 : sb0;

    sb1.append(sb0.toString())
       .append(space)
       .append(1);

    __noinline__consume(sb);

    return sb1.toString();                  // not applied, sb0 must not be used after sb0.toString() appended
}

let sb0_global = new StringBuilder("0");
function unsupported_chain_concat13(): String {
    let sb1 = new StringBuilder();
    sb1.append(sb0_global.toString())
       .append(space)
       .append(1);

    return sb1.toString();                  // not applied, sb0 must be local instance
}

function partially_supported_chain_concat14(a: String, b: String, c: String, d: String): String {
    let ab = a + b;
    let cd = c + d;                                 // not applied, due to 'cd' appended to result is not the first append
    return ab + cd;                                 // applied, 3 SBs merged into 1 SB + concat2
}

function supported_chain_concat15(a: String, b: String, c: String, d: String): String {
    let str = a + b;
    str += c;
    str += d;                                       // applied, 3 SBs merged into 1 SB
    return str;
}

function partially_supported_chain_concat16(a: String, b: String, c: String, d: String): String {
    let abcd = a + b;
    abcd += c + d;                                  // not applied, due to 'c+d' appended to result is not the first append
    return abcd;                                    // applied, 3 SBs merged into 1 SB + concat2
}

function unsupported_usage_chain_concat17(a: String): String {
    let sb = new StringBuilder();
    sb.append(a);
    let result = sb.toString();
    sb.append(a);
    return result;                                  // not applied, sb must not be used after sb.toString() appended
}

function unsupported_usage_chain_concat18(a: String): String {
    let sb = new StringBuilder();
    sb.append(a);
    let result = sb.toString();
    sb.append(a);
    return result.concat();                         // not applied, sb must not be used after sb.toString() appended
}

//! CHECKER       AOT IR Builder, check String Builders merging (chain_concat19 only, with --compiler-inlining=true)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*chain_concat19' --compiler-inlining=true"
//! RUN           entry: "ets_string_builder_merge.ETSGLOBAL::main"

function chain_concat19(): void {
    const _ = "_";
    const a = "a";
    const b = "b";
    const c = "c";

    let sb_a = new StringBuilder();
    sb_a.append(_)
    sb_a.append(a)
    let _a = sb_a.toString()

    let sb_ab = new StringBuilder();
    sb_ab.append(_a)
    sb_ab.append(b)
    let _ab = sb_ab.toString()

    let sb_abc = new StringBuilder();
    sb_abc.append(_ab)
    sb_abc.append(c)
    let _abc = sb_abc.toString()

    arktest.assertEQ(_a, "_a");
    arktest.assertEQ(_ab, "_ab");
    arktest.assertEQ(_abc, "_abc");
}

function main() {
    arktest.assertEQ(chain_concat0("a", 4, "b", 3), "_aaaa_bbb_", "Wrong result at chain_concat0");
    arktest.assertEQ(chain_concat1("a", 4, "b", 3, "c", 5), "_aaaa_bbb_ccccc_", "Wrong result at chain_concat1");
    arktest.assertEQ(chain_concat2("a", 4, "b", 3, "c", 5, "d", 2), "_aaaa_bbb_ccccc_dd_", "Wrong result at chain_concat2");
    arktest.assertEQ(chain_concat3("a", 4, "b", 3, "c", 5, "d", 2), "_aaaa_bbb_aaaa_bbb_ccccc_dd_", "Wrong result at chain_concat3");

    arktest.assertEQ(supported_chain_concat5(), "012 012", "Wrong result at supported_chain_concat5");
    arktest.assertEQ(supported_chain_concat6(), " abcΣΨΩ0123456789 true false A 57 127 32767 128934675 701234987654321 string [object Object] 1 1 1  abcΣΨΩ0123456789 true false A 57 127 32767 128934675 701234987654321 string [object Object] 1 1 1 ", "Wrong result at supported_chain_concat6");

    arktest.assertEQ(unsupported_append_chain_concat7(), "0 0", "Wrong result at unsupported_append_chain_concat7");
    arktest.assertEQ(unsupported_constructor_chain_concat8(), "00", "Wrong result at unsupported_constructor_chain_concat8");
    arktest.assertEQ(unsupported_usage_chain_concat9(), "0", "Wrong result at unsupported_usage_chain_concat9");

    arktest.assertEQ(unsupported_chain_concat10(false), "210 012", "Wrong result at unsupported_chain_concat10(false)");
    arktest.assertEQ(unsupported_chain_concat10(true), "012 012", "Wrong result at unsupported_chain_concat10(true)");

    arktest.assertEQ(unsupported_chain_concat11(false), "0 2", "Wrong result at unsupported_chain_concat11(false)");
    arktest.assertEQ(unsupported_chain_concat11(true), "1 2", "Wrong result at unsupported_chain_concat11(true)");

    arktest.assertEQ(unsupported_chain_concat12(false), "0 1", "Wrong result at unsupported_chain_concat12(false)");
    arktest.assertEQ(unsupported_chain_concat12(true), "0 1", "Wrong result at unsupported_chain_concat12(true)");

    arktest.assertEQ(unsupported_chain_concat13(), "0 1", "Wrong result at unsupported_chain_concat13()");

    arktest.assertEQ(partially_supported_chain_concat14("ab", "c", "d", "e"), "abcde", "Wrong result at partially_supported_chain_concat14");
    arktest.assertEQ(supported_chain_concat15("ab", "c", "d", "e"), "abcde", "Wrong result at supported_chain_concat15");
    arktest.assertEQ(partially_supported_chain_concat16("ab", "c", "d", "e"), "abcde", "Wrong result at partially_supported_chain_concat16");
    arktest.assertEQ(unsupported_usage_chain_concat17("abc"), "abc", "Wrong result at unsupported_usage_chain_concat17");
    arktest.assertEQ(unsupported_usage_chain_concat18("abc"), "abc", "Wrong result at unsupported_usage_chain_concat18");

    chain_concat19();
}

//! CHECKER       Unsupported concat pattern AOT (IBLDF4/22718)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_chain_concat20' --compiler-inlining=true"
//! RUN           entry: "ets_string_builder_merge.ETSGLOBAL::main"

//! CHECKER       Unsupported concat pattern JIT (IBLDF4/22718)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*bugfix_chain_concat20'", entry: "ets_string_builder_merge.ETSGLOBAL::bugfix_chain_concat20"

function bugfix_chain_concat20(): void
{
    let a = "a";
    let b = "b";
    let abc = a + b + "c";
    let abcd = abc + "d";
    let abce = abc + "e";

    arktest.assertEQ(abc, "abc");
    arktest.assertEQ(abcd, "abcd");
    arktest.assertEQ(abce, "abce");
}

//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO       options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"

//! CHECKER       BCO, check String Builders merging
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,5
//! IN_BLOCK      /prehead \(loop 2\)/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 2, depth 1/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /prehead \(loop 1\)/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,1
//! IN_BLOCK      /prehead \(loop 2\)/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2, depth 1/
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead \(loop 1\)/
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1/
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",7
//! INST_COUNT    /StringBuilder::append/,7
//! INST_COUNT    /StringBuilder::toString/,7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",9
//! INST_COUNT    /StringBuilder::append/,9
//! INST_COUNT    /StringBuilder::toString/,9
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,9
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chain_concat3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    /StringBuilder::append/,14
//! INST_COUNT    /StringBuilder::toString/,14
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,14
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,8
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,7
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat6"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,61
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,60
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_append_chain_concat7"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_constructor_chain_concat8"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat9"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat10"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat11"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",3
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat12"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_chain_concat13"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partially_supported_chain_concat14"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supported_chain_concat15"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partially_supported_chain_concat16"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat17"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupported_usage_chain_concat18"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1

//! CHECKER       INTERP-IRTOC, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='ets_string_builder_merge.ETSGLOBAL::main' --compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_builder_merge.ETSGLOBAL::main"

//! CHECKER       INTERP-CPP, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='ets_string_builder_merge.ETSGLOBAL::main' --compiler-enable-jit=false --interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_builder_merge.ETSGLOBAL::main"

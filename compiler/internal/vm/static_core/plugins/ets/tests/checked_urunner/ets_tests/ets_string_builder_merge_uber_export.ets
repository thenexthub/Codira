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
//! METHOD        "ets_string_builder_merge_uber_export.ETSGLOBAL::uber_chain_concat4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",64
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",64
//! INST_COUNT    "Intrinsic.StdCoreSbToString",64
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//!
//! RUN           entry: "ets_string_builder_merge_uber_export.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*chain_concat[0-9]+.*' --compiler-inlining=false", entry: "ets_string_builder_merge_uber_export.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_merge_uber_export.ETSGLOBAL::uber_chain_concat4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",64
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",64
//! INST_COUNT    "Intrinsic.StdCoreSbToString",64
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5

export function uber_chain_concat4(do_exit: boolean, a: String, n: int, b: String, m: int, c: String, l: int, d: String, k: int,
                            cond0: boolean, cond1: boolean, do_throw0: boolean, do_throw1: boolean): String {
    /*
        This test has the following data flow (very-very briefly):
            0     3'
           / \   /
          0   1 3'
          |   |/
          0   1
           \ / \
            0   2
            |   |
            0   2
             \ / \
              0   3
              |   |
              0   3
               \ /
                0
        It contans 5 individial chains of SBs (0, 1, 2, 3 and 3').
        Each of them can be collapsed into a single SB.
    */

    if (do_exit)
        return "";

    let str0: String = "";
    let str1: String = "";
    let str2: String = "";
    let str3: String = "";
    str0 += "_";
    for (let i = 0; i < n; ++i) {
        str0 += a;
    }
    str0 += "_";
    str1 += str0;
    str3 += "_";
    if (cond0) {
        for (let i = 0; i < m; ++i) {
            try {
                if (do_throw0)
                    throw new Error();
                str0 += b;
                str1 += c;
                str3 += d;
                if (do_throw1)
                    throw new Error();
            }
            catch(ex) {
                str0 += a;
                str1 += b;
                str3 += c;
            }
            finally {
                str0 += d;
                str1 += a;
                str3 += b;
            }
        }
    }
    else {
        for (let i = 0; i < m; ++i) {
            try {
                if (do_throw0)
                    throw new Error();
                str0 += c;
                str1 += d;
                str3 += a;
                if (do_throw1)
                    throw new Error();
            }
            catch(ex) {
                str0 += b;
                str1 += c;
                str3 += d;
            }
            finally {
                str0 += a;
                str1 += b;
                str3 += c;
            }
        }
    }
    str0 += str1;
    str0 += "_";
    str1 += str3;
    str1 += "_";
    str2 += str1;
    for (let i = 0; i < l; ++i) {
        if (cond1) {
            str0 += d;
            str2 += a;
        }
        else {
            str0 += b;
            str2 += c;
        }
    }
    str0 += str2;
    str0 += "_";
    str3 += str2;
    str3 += "_";
    if (cond0) {
        for (let i = 0; i < k; ++i) {
            if (cond1) {
                str0 += d;
                str3 += a;
            }
            else {
                str0 += b;
                str3 += c;
            }
        }
    }
    else {
        for (let i = 0; i < k; ++i) {
            if (cond1) {
                try {
                    if (do_throw0)
                        throw new Error();
                    str0 += d;
                    str3 += a;
                    if (do_throw1)
                        throw new Error();
                }
                catch(ex) {
                    str0 += b;
                    str3 += c;
                }
                finally {
                    str0 += d;
                    str3 += a;
                }
            }
            else {
                try {
                    if (do_throw0)
                        throw new Error();
                    str0 += b;
                    str3 += c;
                    if (do_throw1)
                        throw new Error();
                }
                catch(ex) {
                    str0 += d;
                    str3 += a;
                }
                finally {
                    str0 += b;
                    str3 += c;
                }
            }
        }
    }
    str0 += str3;
    str0 += "_";
    return str0;    // applied, 64 SBs + 128 append calls merged into 5 SBs + 5 append2 calls + 5 append calls
}

function main() {
    uber_chain_concat4(true, "", 0, "", 0, "", 0, "", 0, false, false, false, false);
}

//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO       options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"

//! CHECKER       BCO, check String Builders merging
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD        "ets_string_builder_merge_uber_export.ETSGLOBAL::uber_chain_concat4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",64
//! INST_COUNT    /StringBuilder::append/,64
//! INST_COUNT    /StringBuilder::toString/,64
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",59
//! INST_COUNT    /StringBuilder::append/,64
//! INST_COUNT    /StringBuilder::toString/,59

//! CHECKER       INTERP-IRTOC, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_builder_merge_uber_export.ETSGLOBAL::main"

//! CHECKER       INTERP-CPP, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-enable-jit=false --interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_builder_merge_uber_export.ETSGLOBAL::main"

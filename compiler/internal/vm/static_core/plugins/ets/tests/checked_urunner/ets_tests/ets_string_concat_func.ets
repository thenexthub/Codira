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

//! CHECKER       AOT IR Builder, check String.concat function
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*concat[0-9]+'"
//!
//! RUN           entry: "ets_string_concat_func.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String.concat function
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*concat[0-9]+.*'", entry: "ets_string_concat_func.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat0"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat1"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! PASS_AFTER    "Escape"
//! INST_NOT      "NewArray"
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat2"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! PASS_AFTER    "Escape"
//! INST_NOT      "NewArray"
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat3"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! PASS_AFTER    "Escape"
//! INST_NOT      "NewArray"
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat4"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat5"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,6
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat6"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! PASS_AFTER    "Escape"
//! INST_NOT      "NewArray"
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat7"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST          "StoreArray"
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,6
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! INST_NOT      "StoreArray"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//!
//! METHOD        "ets_string_concat_func.ETSGLOBAL::concat8"
//! PASS_BEFORE   "BranchElimination"
//! INST          /String::concat/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "OptimizeStringConcat"
//! INST_NOT      /String::concat/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1

function concat0(a: string): string {
    return a.concat();                              // applied
}

function concat1(a: string, b: string): string {
    return a.concat(b);                             // applied
}

function concat2(a: string, b: string, c: string): string {
    return a.concat(b, c);                          // applied
}

function concat3(a: string, b: string, c: string, d: string): string {
    return a.concat(b, c, d);                       // applied
}

function concat4(a: string, b: string, c: string, d: string, e: string): string {
    return a.concat(b, c, d, e);                    // applied
}

function concat5(a: string, b: string, c: string, d: string, e: string, f: string): string {
    return a.concat(b, c, d, e, f);                 // applied
}

function concat6(): string {
    let args = ["a", "b", "cde"]
    return "".concat(...args);                      // applied
}

function concat7(): string {
    let args = ["a", "b", "c", "d", "e"]
    return "".concat(...args);                      // applied
}

function concat8(...args: string[]): string {
    return "".concat(...args);                      // applied
}

function main() {
    arktest.assertEQ(concat0("abcde"), "abcde", "Wrong result at concat0")
    arktest.assertEQ(concat1("abcd", "e"), "abcde", "Wrong result at concat1")
    arktest.assertEQ(concat2("abc", "d", "e"), "abcde", "Wrong result at concat2")
    arktest.assertEQ(concat3("ab", "c", "d", "e"), "abcde", "Wrong result at concat3")
    arktest.assertEQ(concat4("a", "b", "c", "d", "e"), "abcde", "Wrong result at concat4")
    arktest.assertEQ(concat5("", "a", "b", "c", "d", "e"), "abcde", "Wrong result at concat5")
    arktest.assertEQ(concat6(), "abcde", "Wrong result at concat6")
    arktest.assertEQ(concat7(), "abcde", "Wrong result at concat7")
    arktest.assertEQ(concat8("ab", "c", "d", "e"), "abcde", "Wrong result at concat8")
}

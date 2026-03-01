/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER       AOT IR Builder, check String concatenation
//! SKIP_IF       @architecture == "arm32"
//! RUN           entry: "ets_string_concat.ETSGLOBAL::main"
//! RUN_PAOC      options: "--compiler-regex='.*concat[0-9]+' --compiler-inlining=false"
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat0"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat1"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat2"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat3"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat4"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat3/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat5"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat8"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat10"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString4/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,0
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat11"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppend/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",1
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat13"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/, 1
//! INST          /Intrinsic.StdCoreSbAppendString /
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/, 1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat14"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/

//! CHECKER       JIT IR Builder, check String concatenation
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*concat[0-9]+.*' --compiler-inlining=false", entry: "ets_string_concat.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat0"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat1"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat2"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat3"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat4"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat3/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat5"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat8"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat10"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat11"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppend/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",1
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat13"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/, 1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/, 1
//!
//! METHOD        "ets_string_concat.ETSGLOBAL::concat14"
//! PASS_BEFORE   "BranchElimination"
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreStringConcat/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendString/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/

function concat0(a: String, b: String): String {
    return a + b;                                   // applied
}

function concat1(a: String, b: String): String {
    let sb = new StringBuilder();
    sb.append(a);
    sb.append(b);
    return sb.toString();                           // applied, IR is equivalent to 'return a + b;'
}

function concat2(a: String, b: String): String {
    let sb = new StringBuilder(a);
    sb.append(b);
    return sb.toString();                           // applied, IR is equivalent to 'return a + b;'
}

let sb: StringBuilder;

function concat3(a: String, b: String): String {
    sb = new StringBuilder();                       // the object is a global variable
    sb.append(a);
    sb.append(b);
    return sb.toString();                           // not applied, IR is not equivalent to 'return a + b;'
}

function concat4(a: String, b: String, c: String): String {
    return a + b + c;                               // applied
}

function concat5(a: String, b: String, c: String, d: String): String {
    return a + b + c + d;                           // applied
}

function concat8(a: String, b: String): String {
    let sb = new StringBuilder();
    sb.append(a);
    if (b != "")
        sb.append(b);
    return sb.toString();                           // not applied, IR is not equivalent to 'return a + b;'
}

function concat10(a: String, b: String, c: String, d: String, e: String): String {
    return a + b + c + d + e;                       // not applied, to many args
}

function concat11(a: String, b: String): String {
    return a + b + 1;                               // not applied, concatenation with int
}

function concat13(): string
{
    let str = "" + StringBuilder.toString(1);       // partially applied, calls StdCoreToStringInt
    return str;
}

function concat14(src: string|undefined): string {
  let dst: string = "" + src;                       // not applied due to instance used outside
                                                    // basic block it was created
  return dst;
}

function main() {
    arktest.assertEQ(concat0("abc", "de"), "abcde", "Wrong result at concat0")
    arktest.assertEQ(concat1("abc", "de"), "abcde", "Wrong result at concat1")
    arktest.assertEQ(concat2("abc", "de"), "abcde", "Wrong result at concat2")
    arktest.assertEQ(concat3("abc", "de"), "abcde", "Wrong result at concat3")
    arktest.assertEQ(concat4("ab", "c", "de"), "abcde", "Wrong result at concat4")
    arktest.assertEQ(concat5("ab", "c", "d", "e"), "abcde", "Wrong result at concat5")
    arktest.assertEQ(concat8("abc", "de"), "abcde", "Wrong result at concat8")
    arktest.assertEQ(concat10("a", "b", "c", "d", "e"), "abcde", "Wrong result at concat10")
    arktest.assertEQ(concat11("abc", "de"), "abcde1", "Wrong result at concat11")
    arktest.assertEQ(concat13(), "1", "Wrong result at concat13")
    arktest.assertEQ(concat14(undefined), "undefined", "Wrong result at concat14")
}

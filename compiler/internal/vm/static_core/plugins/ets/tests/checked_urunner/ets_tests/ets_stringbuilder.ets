/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

//! CHECKER       AOT IR Builder, check StringBuilder::toString replacement
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*toString[0-9]+.*' --compiler-inlining=false"
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString0"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString1"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString2"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /NullCheck/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString3"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString4"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /NullCheck/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString5"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString6"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString7"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString8"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendByte/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString10"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! RUN           entry: "ets_stringbuilder.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check StringBuilder::toString replacement
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*toString[0-9]+.*' --compiler-inlining=false", entry: "ets_stringbuilder.ETSGLOBAL::main"
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString0"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString1"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString2"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /NullCheck/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString3"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString4"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /NullCheck/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString5"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString6"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString7"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString8"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /NullCheck/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbAppendByte/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString10"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString11"
//! PASS_BEFORE   "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1


function toString0(str: String): String {
    return new StringBuilder(str).toString();   // applied
}

function toString1(str: String): String {
    let sb = new StringBuilder(str);
    return sb.toString();                       // applied
}

function toString2(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    sb = sb;
    sb.toString();                              // applied
    sb = sb.append(1);
    return sb.toString();                       // not applied, due to dominating append()-call
}

function doSmth() : void {
}

function doSmthWithSB(sb: StringBuilder) : void {
}

function toString3(str: String): String {
    let sb = new StringBuilder(str);            // not removed, due to potential inlining of doSmth with potential deoptimize instructions
    sb.toString();                              // applied
    doSmth();
    return sb.toString();                       // applied
}

function toString4(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    doSmthWithSB(sb);
    return sb.toString();                       // not applied, due to dominating doSmthWithSB()-call
}

function toString5(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    let sb2 = sb;
    doSmthWithSB(sb2);
    return sb.toString();                       // not applied, due to dominating doSmthWithSB()-call
}

class A {
    public sb: StringBuilder = new StringBuilder();
}

function doSmthWithA(a: A) : void {
}

function toString6(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    let a = new A;
    a.sb = sb;
    doSmthWithA(a);
    return sb.toString();                       // not applied, due to dominating StoreObject-instruction (a.sb = sb)
}

let sb: StringBuilder = new StringBuilder();

function toString7(str: String): String {
    sb = new StringBuilder(str);                // the object is a global variable
    sb.toString();                              // not applied, due to dominating StoreStatic-instruction
    doSmth();
    return sb.toString();                       // not applied, due to dominating StoreStatic-instruction
}

function toString8(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    sb.append(1);
    return sb.toString();                       // not applied, due to dominating append()-call
}

function toString9(str: String): String {
    let sb1 = new StringBuilder(str);
    sb1.toString();                             // applied
    let sb2 = new StringBuilder(str);
    sb1.toString();                             // applied
    sb2.toString();                             // applied
    sb1.append(1);
    sb1.toString();                             // not applied, due to dominating append()-call
    sb2.toString();                             // applied
    sb2.append(1);
    return sb2.toString();                      // not applied, due to dominating append()-call
}

function toString10(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    let strings: FixedArray<string> = ["1"]               // insert NegativeCheck, BoundsCheck, RefTypeCheck instructions
    sb.toString();                              // applied
    sb.append(strings[0]);
    return sb.toString();                       // not applied, due to dominating append()-call
}

function toString11(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();                              // applied
    if (str.getLength() > 0)
        sb.append(1 as int);
    return sb.toString();                       // not applied, due to dominating append()-call
}

function main(): int {
    arktest.assertEQ(toString0("abcde"), "abcde", "Wrong at toString0")
    arktest.assertEQ(toString1("abcde"), "abcde", "Wrong at toString1")
    arktest.assertEQ(toString2("abcde"), "abcde1", "Wrong at toString2")
    arktest.assertEQ(toString3("abcde"), "abcde", "Wrong at toString3")
    arktest.assertEQ(toString4("abcde"), "abcde", "Wrong at toString4")
    arktest.assertEQ(toString5("abcde"), "abcde", "Wrong at toString5")
    arktest.assertEQ(toString6("abcde"), "abcde", "Wrong at toString6")
    arktest.assertEQ(toString7("abcde"), "abcde", "Wrong at toString7")
    arktest.assertEQ(toString8("abcde"), "abcde1", "Wrong at toString8")
    arktest.assertEQ(toString9("abcde"), "abcde1", "Wrong at toString9")
    arktest.assertEQ(toString10("abcde"), "abcde1", "Wrong at toString10")
    arktest.assertEQ(toString11("abcde"), "abcde1", "Wrong at toString10")
    return 0;
}

// Checker below based on plugins/ets/sdk/api/@ohos.url.ets
// Expected successful compilation

//! CHECKER       AOT IR Builder, Bugfix 25869
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*encodePercentEncoding.*' --compiler-inlining=false --compiler-check-final=false"

const HASH_TABLE: Array<string> = new Array<string>(256);
const NO_ESCAPE_TABLE = new Int8Array(256);

function encodePercentEncoding(str: string) {
    let outVal = '';

    const len = str.length;
    let i = 0;
    const UNICODE_0x80 = 0x80;

    outer:
    for (; i < len; i++) {
        let c = str.charCodeAt(i).toInt();

        while (c < UNICODE_0x80) {
            if (NO_ESCAPE_TABLE[c] !== 1) {
                outVal += HASH_TABLE[c];
            }
            if (++i == len)
                break outer;
        }
    }
    return outVal;
}

//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO       options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"

//! CHECKER       BCO, check StringBuilder::toString replacement
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString0"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString1"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString2"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString3"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString4"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString5"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString6"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT_NOT /SaveState/
//! INST_NEXT_NOT /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString7"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString8"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::append/
//! INST_NEXT     /SaveState/
//! INST_NEXT     /StringBuilder::toString/
//!
//! METHOD        "ets_stringbuilder.ETSGLOBAL::toString10"
//! PASS_AFTER    "BranchElimination"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::toString/,1

//! CHECKER       INTERP-IRTOC, check StringBuilder::toString replacement
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_stringbuilder.ETSGLOBAL::main"

//! CHECKER       INTERP-CPP, check StringBuilder::toString replacement
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-enable-jit=false --interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_stringbuilder.ETSGLOBAL::main"

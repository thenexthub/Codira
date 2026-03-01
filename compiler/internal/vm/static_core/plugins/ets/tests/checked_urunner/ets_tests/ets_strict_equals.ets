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

type SomeRef = Object | null | undefined;

// Prevents comparison elimination by frontend type inferrence
function getfalse(): boolean { return false }
function getnull(): SomeRef { return null }
function getundef(): SomeRef { return undefined }
function id(x: SomeRef) { return x }
function getobj(): SomeRef { return new Object(); }

//! CHECKER      AOT ets.strictequals peepholes
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*(equals_).*'"
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_dce"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_cse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /Intrinsic.CompilerEtsStrictEquals/, 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   /Intrinsic.CompilerEtsStrictEquals/, 1
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_same_inputs"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_undefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_null_undefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_reftyped_input"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_union_input"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strict_equals_object_input"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//!
//! RUN          entry: "ets_strict_equals.ETSGLOBAL::main"

function strict_equals_dce(x: SomeRef, y: SomeRef) {
    return getfalse() && x === y;
}
function strict_equals_cse(x: SomeRef, y: SomeRef) {
    return id(x) === id(y) && id(y) === id(x); // commutative
}
function strict_equals_same_inputs(x: SomeRef) {
    return id(x) === id(x);
}
function strict_equals_null(x: SomeRef) {
    return getnull() === x;
}
function strict_equals_undefined(x: SomeRef) {
    return x === getundef();
}
function strict_equals_null_undefined() {
    return getnull() === getundef();
}
function strict_equals_reftyped_input() {
    return getobj() === getobj();
}
function strict_equals_union_input(x: double | string) {
    return x === x;
}
function strict_equals_object_input(x: SomeRef) {
    return x === x;
}

function main() {
    let a = new Int(123);
    let b = new Int(123);

    arktest.assertTrue(!strict_equals_dce(a, b));
    arktest.assertTrue(strict_equals_cse(a, b));
    arktest.assertTrue(strict_equals_same_inputs(a));
    arktest.assertTrue(strict_equals_null(null) && !strict_equals_null(undefined) && !strict_equals_null(a));
    arktest.assertTrue(strict_equals_undefined(undefined) && !strict_equals_undefined(null) && !strict_equals_undefined(a));
    arktest.assertTrue(!strict_equals_null_undefined());
    arktest.assertTrue(!strict_equals_reftyped_input());
    arktest.assertTrue(!strict_equals_union_input(NaN));

    let objNaN: object = NaN;
    arktest.assertTrue(!strict_equals_object_input(objNaN));
}

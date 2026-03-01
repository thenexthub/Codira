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

// Prevents comparison elimination by frontend type inferrence
function getfalse(): boolean { return false }
function getnull(): Any { return null }
function getundef(): Any { return undefined }
function id(x: Any) { return x }
function getobj(): Any { return new Object(); }

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER      AOT ets.equals peepholes
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*(equals_).*'"
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_dce"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_cse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /Intrinsic.CompilerEtsEquals/, 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   /Intrinsic.CompilerEtsEquals/, 1
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_same_inputs"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsEquals/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_undefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_null_undefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_reftyped_input"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_union_input"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsEquals/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equals_object_input"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsEquals/
//!
//! RUN          entry: "ets_equals.ETSGLOBAL::main"

function equals_dce(x: Any, y: Any) {
    return getfalse() && x == y;
}
function equals_cse(x: Any, y: Any) {
    return id(x) == id(y) && id(y) == id(x); // commutative
}
function equals_same_inputs(x: Any) {
    return id(x) == id(x);
}
function equals_null(x: Any) {
    return getnull() == x;
}
function equals_undefined(x: Any) {
    return x == getundef();
}
function equals_null_undefined() {
    return getnull() == getundef();
}
function equals_reftyped_input() {
    return getobj() == getobj();
}
function equals_union_input(x: double | string) {
    return x == x;
}
function equals_object_input(x: Any) {
    return x == x;
}

function main() {
    let a = new Int(123);
    let b = new Int(123);

    arktest.assertTrue(!equals_dce(a, b));
    arktest.assertTrue(equals_cse(a, b));
    arktest.assertTrue(equals_same_inputs(a));
    arktest.assertTrue(equals_null(null) && equals_null(undefined) && !equals_null(a));
    arktest.assertTrue(equals_undefined(undefined) && equals_undefined(null) && !equals_undefined(a));
    arktest.assertTrue(equals_null_undefined());
    arktest.assertTrue(!equals_reftyped_input());
    arktest.assertTrue(!equals_union_input(NaN));

    let objNaN: object = NaN;
    arktest.assertTrue(!equals_object_input(objNaN));
}

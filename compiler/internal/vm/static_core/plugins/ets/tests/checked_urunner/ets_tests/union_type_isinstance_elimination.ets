/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER         Check object type elimination for union type (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*checkElimination.*'", entry: "union_type_isinstance_elimination.ETSGLOBAL::main"
//!
//! METHOD          "union_type_isinstance_elimination.ETSGLOBAL::checkElimination"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "IsInstance", 3
//! PASS_AFTER      "Codegen"
//! INST_COUNT      "IsInstance", 2
//!
//! METHOD          "union_type_isinstance_elimination.ETSGLOBAL::checkEliminationForBaseType"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Codegen"
//! INST_NOT        "IsInstance"
//!
//! METHOD          "union_type_isinstance_elimination.ETSGLOBAL::checkEliminationForEnumArrayTemplateTypes"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      "IsInstance", 3
//! PASS_AFTER      "Codegen"
//! INST_COUNT      "IsInstance", 1
//!
//! METHOD          "union_type_isinstance_elimination.ETSGLOBAL::checkEliminationForInterfaces"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "IsInstance"

enum Color {Red = 1, Green = 2, Blue = 3}

function checkElimination() {
    arktest.assertEQ(Color.Red instanceof int | string, false);
    arktest.assertEQ(null instanceof int | string, false);
    arktest.assertEQ('123' instanceof int | string, true);
}

class B {}
class D extends B {}

function checkEliminationForBaseType() {
    // CC-OFFNXT(G.DCL.06) for test purposes
    arktest.assertEQ(new Object() instanceof Int | D, false);
    arktest.assertEQ(new Int(123) instanceof Int | D, true);
    arktest.assertEQ(new D() instanceof Int | D, true);
    arktest.assertEQ(new B() instanceof Int | D, false);
}

enum Size { S = 'small', M = 'medium', L = 'large' }
class A<T> {}

function checkEliminationForEnumArrayTemplateTypes() {
    let x: 'a' | 'b' | int | Error = 'a';
    arktest.assertEQ(x instanceof string | int | Error, true);
    let y: int | Color | Size = 1;
    arktest.assertEQ(y instanceof Color | Size, false);
    let z: Promise<number> | string[][] | undefined | A<int> | Record<string, number> = [['123']];
    arktest.assertEQ(z instanceof Promise | Array | undefined | A | Record, true);
}

interface I1<U, V> {}
interface I2<U, V> {}
interface II1<U, V> extends I1<U, V>, I2<U, V> {}
interface II2<U = bigint, V = undefined> extends I1<U, V>, I2<U, V> {}
interface III1<U = int, V = null> extends II1<U, V> {}
interface III2<U = number, V = string> extends II1<U, V> {}
class H<U = bigint, V = undefined> implements II2<U, V>  {}

function checkEliminationForInterfaces() {
    let x = new H();
    arktest.assertEQ(x instanceof II2, true);
    arktest.assertEQ(x instanceof III1 | III2 | II2, true);
    arktest.assertEQ(x instanceof II1 | bigint, false);
}

function main(): void {
    checkElimination();
    checkEliminationForBaseType();
    checkEliminationForEnumArrayTemplateTypes();
    checkEliminationForInterfaces();
}

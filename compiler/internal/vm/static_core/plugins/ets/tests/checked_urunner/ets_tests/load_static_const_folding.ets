/**
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

//! CHECKER         Remove LoadStatic for static readonly fields
//! RUN             force_jit: true, options: "--compiler-regex='.*test.*'", entry: "load_static_const_folding.ETSGLOBAL::main", return: 0
//! METHOD          "load_static_const_folding.ETSGLOBAL::testInt"
//! PASS_AFTER      "Peepholes"
//! INST            /Constant\s+20 ->.+/
//! PASS_AFTER      "Cleanup"
//! INST_NOT        "LoadStatic"
//! INST            /Constant\s+20 ->.+/
//! INST_COUNT      "Constant", 1
//!
//! METHOD          "load_static_const_folding.ETSGLOBAL::testFloat"
//! PASS_AFTER      "Peepholes"
//! INST            /Constant\s+10 ->.+/
//! PASS_AFTER      "Cleanup"
//! INST_NOT        "LoadStatic"
//! INST            /Constant\s+10 ->.+/
//! INST_COUNT      "Constant", 1

class ClassWithStaticFloatField {
    public static readonly STATIC_FIELD: number = 10
}

class ClassWithStaticIntField {
    public static readonly STATIC_FIELD: int = 20
}

function testInt(m: number): number {
    return m * ClassWithStaticIntField.STATIC_FIELD
}

function testFloat(m: number): number {
    return m * ClassWithStaticFloatField.STATIC_FIELD
}

function main(): int {
    // Classes should be loaded and inited
    // for optimization to work
    const f = new ClassWithStaticFloatField
    const i = new ClassWithStaticIntField
    if (testInt(42) != 840) {
        return 1
    }
    if (testFloat(69) != 690) {
        return 1
    }
    return 0
}


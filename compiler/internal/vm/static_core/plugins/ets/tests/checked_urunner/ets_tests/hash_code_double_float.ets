
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

//! CHECKER           AOT: Compare getting hash code values
//! SKIP_IF           @architecture == "arm32"
//! RUN_PAOC          options: "--compiler-regex='.*test.*'", entry: "hash_code_double_float.ETSGLOBAL::main"
//!
//! METHOD            "hash_code_double_float.ETSGLOBAL::testHashCodeByValueDouble"
//! PASS_AFTER        "IrBuilder"
//! INST              "Intrinsic.StdRuntimeGetHashCodeByValue"
//!
//! METHOD            "hash_code_double_float.ETSGLOBAL::testHashCodeByValueFloat"
//! PASS_AFTER        "IrBuilder"
//! INST              "Intrinsic.StdRuntimeGetHashCodeByValue"

//! CHECKER           JIT: Compare getting hash code values
//! RUN               force_jit: true, options: "--compiler-regex='.*test.*'", entry: "hash_code_double_float.ETSGLOBAL::main"
//!
//! METHOD            "hash_code_double_float.ETSGLOBAL::testHashCodeByValueDouble"
//! PASS_AFTER        "IrBuilder"
//! INST              "Intrinsic.StdRuntimeGetHashCodeByValue"
//!
//! METHOD            "hash_code_double_float.ETSGLOBAL::testHashCodeByValueFloat"
//! PASS_AFTER        "IrBuilder"
//! INST              "Intrinsic.StdRuntimeGetHashCodeByValue"

//! CHECKER           INTERP-CPP: Compare getting hash code values
//! RUN               options: "--interpreter-type=cpp --compiler-enable-jit=false", entry: "hash_code_double_float.ETSGLOBAL::main"

function testHashCodeByValueDouble(value: double) {
    return Runtime.getHashCodeByValue(value).toInt();
}

function testHashCodeByValueFloat(value: float) {
    return Runtime.getHashCodeByValue(value).toInt();
}

function main() {
    arktest.assertEQ(testHashCodeByValueDouble(Double.NaN), testHashCodeByValueDouble(-Double.NaN));
    arktest.assertEQ(testHashCodeByValueFloat(Float.NaN), testHashCodeByValueDouble(-Float.NaN));
}

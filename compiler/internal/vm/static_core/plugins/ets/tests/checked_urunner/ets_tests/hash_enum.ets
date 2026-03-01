
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
//! RUN_PAOC          options: "--compiler-regex='.*test.*'", entry: "hash_enum.ETSGLOBAL::main"
//!
//! METHOD            "hash_enum.ETSGLOBAL::testGetHashCode"
//! PASS_AFTER        "IrBuilder"
//! INST_COUNT        "Intrinsic.StdRuntimeGetHashCodeByValue",3
//! RUN               entry:"hash_enum.ETSGLOBAL::main"

//! CHECKER           JIT: Compare getting hash code values
//! RUN               force_jit: true, options: "--compiler-regex='.*test.*'", entry: "hash_enum.ETSGLOBAL::main"
//!
//! METHOD            "hash_enum.ETSGLOBAL::testGetHashCode"
//! PASS_AFTER        "IrBuilder"
//! INST_COUNT        "Intrinsic.StdRuntimeGetHashCodeByValue",3
//!

//! CHECKER           INTERP-CPP: Compare getting hash code values
//! RUN               options: "--interpreter-type=cpp --compiler-enable-jit=false", entry: "hash_enum.ETSGLOBAL::main"

const CONCAT_LHS = "xyzasadasdsadasaaaa"; // longer than 13 chars
class MyEnum extends BaseEnum<string> {
    constructor(c: string) {
        super(CONCAT_LHS + c)
    }
}
function testGetHashCode() {
  const CONCAT_RHS = Runtime.getHashCodeByValue("123123").toString(); // can't constant-fold
  const x = new MyEnum(CONCAT_RHS);
  const xHash = Runtime.getHashCodeByValue(x)
  const strHash = Runtime.getHashCodeByValue(CONCAT_LHS + CONCAT_RHS)
  arktest.assertEQ(xHash, strHash);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("GenerateGetHashCodeByValue tests")
    suite.addTest("Get hash code of enum class", testGetHashCode)
    return suite.run()
}

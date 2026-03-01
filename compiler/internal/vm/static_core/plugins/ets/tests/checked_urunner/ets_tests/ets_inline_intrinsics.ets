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

//! CHECKER      Inline StringEquals intrinsic
//! SKIP_IF      @architecture == "arm32"
//! RUN          options: "--no-async-jit=true  --compiler-hotness-threshold=4 --compiler-profiling-threshold=2 --compiler-regex='.*equals_obj_obj.*'", entry: "ets_inline_intrinsics.ETSGLOBAL::main"
//! METHOD       "ets_inline_intrinsics.ETSGLOBAL::equals_obj_obj"
//! PASS_AFTER   "IrBuilder"
//! INST         /CallVirtual/
//! INST_NOT     /Intrinsic/
//! PASS_AFTER   "Inline"
//! INST         /LoadImmediate/
//! INST_NEXT    /GetInstanceClass/
//! INST_NEXT    /Compare/
//! INST_NEXT    /DeoptimizeIf INLINE_IC/
//! INST_NEXT    /Intrinsic.StdCoreStringEquals/
//! INST_NOT     /CallVirtual/

//! CHECKER      Not inline StringEquals intrinsic
//! SKIP_IF      @architecture != "arm32"
//! RUN          options: "--no-async-jit=true  --compiler-hotness-threshold=4 --compiler-profiling-threshold=2 --compiler-regex='.*equals_obj_obj.*'", entry: "ets_inline_intrinsics.ETSGLOBAL::main"
//! METHOD       "ets_inline_intrinsics.ETSGLOBAL::equals_obj_obj"
//! PASS_AFTER   "Inline"
//! INST         /CallVirtual/
//! INST_NOT     /Intrinsic/

function equals_obj_obj(lhs: Object, rhs: Object | null): boolean {
    return lhs == rhs;
}

class A {}

function main() {
    let str = "abc";
    let obj = new A();

    for (let i:int = 0; i < 10; ++i) {
        arktest.assertTrue(equals_obj_obj(str, str));
        arktest.assertTrue(!equals_obj_obj("abcd", "abce"));
        arktest.assertTrue(!equals_obj_obj(str, obj));
        arktest.assertTrue(!equals_obj_obj(str, null));
    }
}
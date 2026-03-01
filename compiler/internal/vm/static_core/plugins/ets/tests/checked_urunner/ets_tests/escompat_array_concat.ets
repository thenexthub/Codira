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

//! CHECKER       Check Array.concat uses intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex=std.core.Array::concat", entry: "escompat_array_concat.ETSGLOBAL::testConcatForArray"
//! METHOD        "std.core.Array::concat"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.EscompatArrayIsPlatformArray"
//! INST          "Intrinsic.StdCoreObjectCopyTo"
function testConcatForArray() {
    let a1 = Array.of(1, 2)
    let a2 = Array.of(3, 4)
    let a = a1.concat(a2)
    checkArray(a, Array.of(1, 2, 3, 4))
}

function checkArray(arr: ArrayLike<int>, expected: ArrayLike<int>) {
    if (arr.length != expected.length) {
        throw new Error('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error('Unexpected element with index ' + i + ' act ' + arr[i] + ' exp ' + expected[i])
        }
    }
}

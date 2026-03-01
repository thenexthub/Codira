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

class ExtArray<T> extends Array<T> {
    public constructor(n: int) {
        super(n)
    }
}

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       Array unshift cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=std.core.Array::unshift", entry: "escompat_array_unshift.ETSGLOBAL::testUnshift"
//! METHOD        "std.core.Array::unshift"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.EscompatArrayUnshift"
//! INST_NOT      /Call/
function testUnshift() {
    let a = new ExtArray<int>(2)
    a[0] = 1
    a[1] = 2
    let n = a.unshift(71, 72, 73)
    arktest.assertEQ(5, n)
}

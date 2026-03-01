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

//! CHECKER    JIT: std.core.Array shift operation.
//! SKIP_IF    @architecture == "arm32"
//! RUN        force_jit: true, options: "--compiler-regex='.*test'", entry: "escompat_array_shift.ETSGLOBAL::test"
//! METHOD     "escompat_array_shift.ETSGLOBAL::test"
//! PASS_AFTER "IrBuilder"
//! INST       /CallStatic.*std.core.Array::shift/
//! PASS_AFTER "Inline"
//! INST       /Intrinsic.EscompatObjectFastCopyToUncheckedWithBarriers/
//! INST       /CallStatic.Inlined.*std.core.Array::popImpl/

//! CHECKER    AOT: std.core.Array shift operation.
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--compiler-regex='.*test'"
//! METHOD     "escompat_array_shift.ETSGLOBAL::test"
//! PASS_AFTER "IrBuilder"
//! INST       /CallStatic.*std.core.Array::shift/
//! PASS_AFTER "Inline"
//! INST       /Intrinsic.EscompatObjectFastCopyToUncheckedWithBarriers/
//! INST       /CallStatic.Inlined.*std.core.Array::popImpl/

function checkArray(arr: Array<int>, expected: Array<int>) {
    let len = arr.length;
    arktest.assertEQ(len, expected.length);
    for (let i = 0; i < len; i++) {
        arktest.assertEQ(arr[i], expected[i]);
    }
}

function test(): void {
    let size = 16;
    let arr = new Array<int>(size);
    let expArr = new Array<int>(size - 1);
    for (let i = 0; i < size; ++i) {
        arr[i] = i;
    }
    for (let i = 0; i < size - 1; ++i) {
        expArr[i] = i + 1;
    }
    arr.shift();
    arktest.assertEQ(arr.length, size - 1);
    checkArray(arr, expArr);
}

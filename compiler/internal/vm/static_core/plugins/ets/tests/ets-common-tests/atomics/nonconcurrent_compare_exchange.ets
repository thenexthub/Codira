/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

let buf: ArrayBuffer;
let buf_2: ArrayBuffer;
let arr: Int8Array;
let arr_2: BigUint64Array; 

function testMain() {
    buf = new ArrayBuffer(1);
    buf_2 = new ArrayBuffer(8);
    arr = new Int8Array(buf, 0, 1);
    arr_2 = new BigUint64Array(buf_2, 0, 1);

    for (let i = 1; i < 100; i++) {
        let old = Atomics.compareExchange(arr, 0, i - 1, i)
        arktest.assertEQ(old, i - 1)
    }
    arktest.assertEQ(Atomics.load(arr, 0), 99)

    for (let i = 1; i < 100; i++) {
        let old = Atomics.compareExchange(arr_2, 0, new BigInt(i - 1), new BigInt(i)).getLong()
        arktest.assertEQ(old, i - 1)
    }
    arktest.assertEQ(Atomics.load(arr_2, 0).getLong(), 99)
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics nonconcurrent_compare_exchange tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}
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

function main() {
    {
        let arr = new BigInt64Array(1);
        arr[0] = 0n;
        // add operation
        Atomics.add(arr, 0, 15n);
        let res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        Atomics.add(arr, 0, -30n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // sub operation
        Atomics.sub(arr, 0, 8n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // and operation
        Atomics.and(arr, 0, 4n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // or operation
        Atomics.or(arr, 0, 16n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // xor operation
        Atomics.xor(arr, 0, 1n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
    }
    {
        let arr = new BigUint64Array(1);
        arr[0] = -5n;
        // add operation
        Atomics.add(arr, 0, 15n);
        let res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // and operation
        Atomics.and(arr, 0, 4n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // or operation
        Atomics.or(arr, 0, 16n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
        // xor operation
        Atomics.xor(arr, 0, 1n);
        res = arr[0];
        arktest.assertEQ(Atomics.load(arr, 0), res);
    }
}

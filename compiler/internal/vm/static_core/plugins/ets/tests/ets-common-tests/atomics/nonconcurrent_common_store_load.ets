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

function testMain() {
    {
        let sab = new ArrayBuffer(1024);
        let v = new Int8Array(sab, 1);
        v[8] = 52;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new Int16Array(sab, 2);
        v[8] = 52;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new Int32Array(sab, 4);
        v[8] = 52;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new BigInt64Array(sab, 8);
        v[8] = 52n;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new Uint8Array(sab, 1);
        v[8] = 52;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new Uint16Array(sab, 2);
        v[8] = 52;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new Uint32Array(sab, 4);
        v[8] = 52;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
    {
        let sab = new ArrayBuffer(1024);
        let v = new BigUint64Array(sab, 8);
        v[8] = 52n;
        arktest.assertEQ(sab, v.buffer);
        arktest.assertEQ(v[8], Atomics.load(v, 8));
    }
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics nonconcurrent_common_store_load tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}
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
    let buf = new ArrayBuffer(4);
    let arr = new Int8Array(buf, 0, 1);

    Atomics.store(arr, 0, 10);
    arktest.assertEQ(Atomics.load(arr, 0), 10)

    // isLockFree testing
    arktest.assertTrue(Atomics.isLockFree(1) && Atomics.isLockFree(2))
    arktest.assertTrue(Atomics.isLockFree(4) && Atomics.isLockFree(8))
    arktest.assertTrue(!Atomics.isLockFree(3) && !Atomics.isLockFree(5))
    arktest.assertTrue(!Atomics.isLockFree(6) && !Atomics.isLockFree(7))
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics nonconcurrent_store_load_islockfree tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}
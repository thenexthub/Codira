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

function testMain(): void {
    let buf = new ArrayBuffer(1024);
    let arr: Int32Array = new Int32Array(buf, 0);
    Atomics.store(arr, 0, 0);
    let r = Atomics.wait(arr, 0, 0, 10);
    arktest.assertEQ(r, "timed-out")
}

function testTimeoutZero(): void {
    let buf = new ArrayBuffer(1024);
    let arr: Int32Array = new Int32Array(buf, 0);
    Atomics.store(arr, 0, 0);
    let r = Atomics.wait(arr, 0, 0, 0);
    arktest.assertEQ(r, "timed-out")
}

function testTimeoutNegative(): void {
    let buf = new ArrayBuffer(1024);
    let arr: Int32Array = new Int32Array(buf, 0);
    Atomics.store(arr, 0, 0);
    let r = Atomics.wait(arr, 0, 0, -10);
    arktest.assertEQ(r, "timed-out")
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics nonconcurrent_wait_timeout tests')
    suite.addTest('Function testMain', testMain)
    suite.addTest('Function testTimeoutZero', testTimeoutZero)
    suite.addTest('Function testTimeoutNegative', testTimeoutNegative)
    return suite.run()
}
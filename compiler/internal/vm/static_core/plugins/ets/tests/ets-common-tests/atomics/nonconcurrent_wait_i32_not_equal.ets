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

function test(byteToChange: int) {
    let buf = new ArrayBuffer(4);
    let arr = new Int32Array(buf, 0, 1);
    let bytearr = new Int8Array(buf, 0, 4);

    (Console.getInstance()).println('Testing for byte: ' + byteToChange)
    Atomics.store(bytearr, byteToChange, 1);
    let x = Atomics.wait(arr, 0, 0)
    arktest.assertEQ(x, 'not-equal')
}

function testMain() {
    for (let i = 0; i < 4; i++) {
        test(i)
    }
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics nonconcurrent_wait_i32_not_equal tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}
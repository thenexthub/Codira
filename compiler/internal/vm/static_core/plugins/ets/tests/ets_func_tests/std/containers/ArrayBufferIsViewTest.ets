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

function main(): int {
    const suite = new arktest.ArkTestsuite("Array API tests for SDK");
    suite.addTest("Test ArrayBuffer.isView with DataView", test1IsView);
    suite.addTest("Test ArrayBuffer.isView with TypedArray", test2IsView);
    suite.addTest("Test ArrayBuffer.isView with Illegal Scenes", test3IsView);
    return suite.run(); 
}

class TestData {
    static readonly sample1 = new DataView(new ArrayBuffer(1), 0, 0);
    static readonly expected1 = true;

    static readonly sample2 = new Int8Array();
    static readonly sample3 = new Int16Array();
    static readonly sample4 = new Int32Array();
    static readonly sample5 = new BigInt64Array();
    static readonly sample6 = new Float32Array();
    static readonly sample7 = new Float64Array();
    static readonly sample8 = new Uint8Array();
    static readonly sample9 = new Uint8ClampedArray();
    static readonly sample10 = new Uint16Array();
    static readonly sample11 = new Uint32Array();
    static readonly sample12 = new BigUint64Array();
    static readonly expected2 = true;

    static readonly sample13 = new String();
    static readonly expected3 = false;
}

function test1IsView() {
    let sample = TestData.sample1;
    let res = ArrayBuffer.isView(sample);
    arktest.assertEQ(TestData.expected1, res, `Expected: "${TestData.expected1}", Actual: "${res}"`);
}

function test2IsView() {
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample2), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample2)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample3), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample3)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample4), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample4)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample5), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample5)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample6), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample6)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample7), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample7)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample8), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample8)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample9), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample9)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample10), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample10)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample11), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample11)}"`);
    arktest.assertEQ(TestData.expected2, ArrayBuffer.isView(TestData.sample12), `Expected: "${TestData.expected2}", Actual: "${ArrayBuffer.isView(TestData.sample12)}"`);
}

function test3IsView() {
    arktest.assertEQ(TestData.expected3, ArrayBuffer.isView(TestData.sample13), `Expected: "${TestData.expected3}", Actual: "${ArrayBuffer.isView(TestData.sample13)}"`);
}

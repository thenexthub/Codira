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
    suite.addTest("Array extendTo with negative length ", testExtendToNegLength);
    suite.addTest("Array extendTo with positivie length ", testExtendToPosLength);
    return suite.run();
}

class TestData {
    static readonly src = Array.of<number>(1, 2, 3);
    static readonly arrayLength1: int = -1;
    static readonly initialValue1: number = -1;
    static readonly expected1 = Array.from<number>(TestData.src);
    static readonly arrayLength2: int = 5;
    static readonly initialValue2: number = -1;
    static readonly expected2 = Array.of<number>(1, 2, 3, -1, -1);
    static readonly initialValue3: number = -1;
}

function testExtendToNegLength() {
    try {
        let a = Array.from<number>(TestData.src);
        a.extendTo(TestData.arrayLength1, TestData.initialValue1);
    } catch(error) {
        arktest.assertEQ(error.toString(), 'RangeError: Parameter error.Invalid array length.')
    }
}

function testExtendToPosLength() {
    let a = Array.from<number>(TestData.src);
    a.extendTo(TestData.arrayLength2, TestData.initialValue2);
    let result: boolean = equaled(TestData.expected2, a);
    arktest.assertTrue(result, "Error. Got: " + a + ". Expected: " + TestData.expected2);
}

function equaled(arr1:  Array<number>, arr2:  Array<number>): boolean {
    if (arr1.length != arr2.length) {
        return false;
    }
    for (let i = 0; i < arr1.length; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }
    return true;
}

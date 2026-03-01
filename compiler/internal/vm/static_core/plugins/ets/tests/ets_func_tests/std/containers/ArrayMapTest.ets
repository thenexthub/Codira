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
    suite.addTest("Test Array.map with array length reducing", testmap);
    return suite.run(); 
}

type FuncType = (val: number, index: int, obj: Array<number>) => number;

class TestData {
    static callCnt = 0;
    static readonly srcArr: Array<number> = new Array<number>(1, 2, 3, 4, 5);
    static readonly callbackfn: FuncType = (val: number, index: int, obj: Array<number>): number => {
        TestData.srcArr.length = 2;
        TestData.callCnt++;
        return 1;
    }
    static readonly expected = 2;
}

function testmap() {
    TestData.callCnt = 0;
    let resArr = TestData.srcArr.map(TestData.callbackfn);
    arktest.assertEQ(TestData.expected, TestData.callCnt, `Expected: "${TestData.expected}", Actual: "${TestData.callCnt}"`);
    arktest.assertEQ(TestData.expected, TestData.srcArr.length, `Expected: "${TestData.expected}", Actual: "${TestData.srcArr.length}"`);
}

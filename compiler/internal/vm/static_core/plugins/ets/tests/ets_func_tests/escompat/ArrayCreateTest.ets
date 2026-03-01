/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite('Array API tests for SDK');
    suite.addTest('Array create instance with negative length ', testCreateNegLength);
    suite.addTest('Array create instance with positivie length ', testCreatePosLength);
    return suite.run();
}

class TestData {
    public static readonly arrayLength1: int = -1234;
    public static readonly initialValue1: number = -1;
    public static readonly arrayLength2: int = 3;
    public static readonly initialValue2: number = -1;
    public static readonly expected2 = Array.of<number>(-1, -1, -1);
}

function testCreateNegLength() {
    const expectedError = new NegativeArraySizeError('size = ' + TestData.arrayLength1);
    arktest.expectError((): void => { Array.create<number>(TestData.arrayLength1, TestData.initialValue1) },
        expectedError);
}

function testCreatePosLength() {
    let a = Array.create<number>(TestData.arrayLength2, TestData.initialValue2);
    let result: boolean = equaled(TestData.expected2, a);
    arktest.assertTrue(result, 'Error. Got: ' + a + '. Expected: ' + TestData.expected2);
}

function equaled(arr1: Array<number>, arr2: Array<number>): boolean {
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

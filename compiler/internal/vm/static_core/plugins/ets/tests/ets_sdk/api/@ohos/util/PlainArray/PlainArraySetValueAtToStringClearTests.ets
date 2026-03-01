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

import { PlainArray } from "@ohos.util.PlainArray";
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("PlainArray setValueAt", testSetValueAtValidIndex);
    suite.addTest("PlainArray setValueAt key by boundary index", testSetValueAtBoundaryIndex);
    suite.addTest("PlainArray setValueAt key by exceeded index", testSetValueAtExceededIndex);
    suite.addTest("PlainArray setValueAt with empty container", testSetValueAtWithEmpty);
    suite.addTest("PlainArray toString with non-empty container", testToStringWithNonEmpty);
    suite.addTest("PlainArray toString with empty container", testToStringWithEmpty);
    suite.addTest("PlainArray clear with non-empty container", testClearWithNonEmpty);
    suite.addTest("PlainArray clear with empty container", testClearWithEmpty);

    return suite.run();
}

function initializePlainArray<T>(plainArray: PlainArray<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        plainArray.add(i, sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber6: int = 6;
    static readonly testString1: string = "0:0,1:2,2:2,3:3,4:4";
    static readonly testString2: string = "0:0,1:1,2:2,3:3,4:4";
    static readonly testString3: string = "0:6,1:2,2:2,3:3,4:6";
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testSetValueAtValidIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    plainArray.setValueAt(TestData.testNumber1, TestData.testNumber1);
    arktest.assertEQ(plainArray.toString(), TestData.testString2, "Set successful and other elements should not change");
}

function testSetValueAtBoundaryIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    plainArray.setValueAt(0, TestData.testNumber6);
    plainArray.setValueAt(TestData.forNumber2.length - TestData.testNumber1, TestData.testNumber6);
    arktest.assertEQ(plainArray.toString(), TestData.testString3, "Set successful and other elements should not change");
}

function testSetValueAtExceededIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: -1` &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    let exceptionCheckTwo = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: 6` &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {plainArray.setValueAt(-TestData.testNumber1, TestData.testNumber6)}, exceptionCheck);
    arktest.expectThrow(() => {plainArray.setValueAt(TestData.testNumber6, TestData.testNumber6)}, exceptionCheckTwo);
}

function testSetValueAtWithEmpty() {
    let plainArray = new PlainArray<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`;
        }
        return false;
    }

    arktest.expectThrow(() => {plainArray.setValueAt(0, TestData.testNumber2)}, exceptionCheck);
}

function testToStringWithNonEmpty() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.toString(), TestData.testString1, "The result should be correct string representation");
}

function testToStringWithEmpty() {
    let plainArray = new PlainArray<double>();
    arktest.assertEQ(plainArray.toString(), "", "The result should be correct string representation");
}

function testClearWithNonEmpty() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.length, TestData.forNumber2.length,
        "The plainArray length should be same as TestData.forNumber2 length");
    plainArray.clear();
    arktest.assertEQ(plainArray.length, 0, "The plainArray length should be 0 after clear");
    arktest.assertTrue(plainArray.isEmpty(), "The plainArray should be empty after clear");
}

function testClearWithEmpty() {
    let plainArray = new PlainArray<double>();
    arktest.assertEQ(plainArray.length, 0, "The plainArray length should be 0");
    plainArray.clear();
    arktest.assertEQ(plainArray.length, 0, "The plainArray length should be 0 after clear");
}

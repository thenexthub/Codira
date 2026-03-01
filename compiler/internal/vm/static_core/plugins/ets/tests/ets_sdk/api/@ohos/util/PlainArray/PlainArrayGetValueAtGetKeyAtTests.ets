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

    suite.addTest("PlainArray getKeyAt", testGetKeyAtValidIndex);
    suite.addTest("PlainArray getKeyAt key by boundary index", testGetKeyAtBoundaryIndex);
    suite.addTest("PlainArray getKeyAt key by exceeded index", testGetKeyAtExceededIndex);
    suite.addTest("PlainArray getKeyAt with empty container", testGetKeyAtIndexOfEmpty);
    suite.addTest("PlainArray getValueAt", testGetValueAtValidIndex);
    suite.addTest("PlainArray getValueAt key by boundary index", testGetValueAtBoundaryIndex);
    suite.addTest("PlainArray getValueAt key by exceeded index", testGetValueAtExceededIndex);
    suite.addTest("PlainArray getValueAt with empty container", testGetValueAtWithEmpty);

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
    static readonly testNumber4: int = 4;
    static readonly testNumber6: int = 6;
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testGetKeyAtValidIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.getKeyAt(TestData.testNumber1), TestData.testNumber1,
        "The plainArray should return the correct key at the specified index");
}

function testGetKeyAtBoundaryIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.getKeyAt(0), 0, "The plainArray should return the correct key at index 0");
    arktest.assertEQ(plainArray.getKeyAt(TestData.forNumber2.length - TestData.testNumber1), TestData.testNumber4,
        "The plainArray should return the correct key at index length - 1");
}

function testGetKeyAtExceededIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.getKeyAt(-TestData.testNumber1), -1);
    arktest.assertEQ(plainArray.getKeyAt(TestData.testNumber6), -1);
}

function testGetKeyAtIndexOfEmpty() {
    let plainArray = new PlainArray<double>();
    arktest.assertEQ(plainArray.getKeyAt(0), -1);
}

function testGetValueAtValidIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.getValueAt(TestData.testNumber1), TestData.testNumber2,
        "The plainArray should return the correct value at the specified index");
}

function testGetValueAtBoundaryIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.getValueAt(0), 0, "The plainArray should return the correct value at index 0");
    arktest.assertEQ(plainArray.getValueAt(TestData.forNumber2.length - TestData.testNumber1), TestData.testNumber4,
        "The plainArray should return the correct value at index length - 1");
}

function testGetValueAtExceededIndex() {
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

    arktest.expectThrow(() => {plainArray.getValueAt(-TestData.testNumber1)}, exceptionCheck);
    arktest.expectThrow(() => {plainArray.getValueAt(TestData.testNumber6)}, exceptionCheckTwo);
}

function testGetValueAtWithEmpty() {
    let plainArray = new PlainArray<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`
        }
        return false;
    }

    arktest.expectThrow(() => {plainArray.getValueAt(0)}, exceptionCheck);
}

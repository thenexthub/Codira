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

import { PlainArray } from "@ohos.util.PlainArray";
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("PlainArray remove with exist key", testRemoveWithExistKey);
    suite.addTest("PlainArray remove with non-exist key", testRemoveWithNonExistKey);
    suite.addTest("PlainArray remove empty container", testRemoveWithEmpty);
    suite.addTest("PlainArray removeAt by valid index", testRemoveAtValidIndex);
    suite.addTest("PlainArray removeAt by boundary index", testRemoveAtBoundaryIndex);
    suite.addTest("PlainArray removeAt by exceeded index", testRemoveAtExceededIndex);
    suite.addTest("PlainArray removeAt empty container", testRemoveAtWithEmpty);
    suite.addTest("PlainArray removeRangeFrom by valid range", testRangeFromValidRange);
    suite.addTest("PlainArray removeRangeFrom by exceeded range", testRangeFromExceededRange);
    suite.addTest("PlainArray removeRangeFrom by exceeded index", testRangeFromExceededIndex);
    suite.addTest("PlainArray removeRangeFrom empty container", testRangeFromWithEmpty);
    suite.addTest("PlainArray removeRangeFrom with double", testRangeFromWithDouble);

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
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber6: int = 6;
    static readonly testString1: string = "0:0,1:2,2:2,4:4";
    static readonly testString2: string = "0:0,1:2,2:2,3:3,4:4";
    static readonly testString3: string = "1:2,2:2,3:3";
    static readonly testString4: string = "0:0,1:2,4:4";
    static readonly testString5: string = "0:0,1:2";
    static readonly testDouble: double = 7.5;
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testRemoveWithExistKey() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.remove(TestData.testNumber3), TestData.testNumber3, "The result should be removed value");
    arktest.assertEQ(plainArray.toString(), TestData.testString1, "The removed sequence is normal");
}

function testRemoveWithNonExistKey() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.remove(TestData.testNumber6), undefined,
        "The result should be undefined when key is not exist");
    arktest.assertEQ(plainArray.toString(), TestData.testString2, "The sequence should not change");
}

function testRemoveWithEmpty() {
    let plainArray = new PlainArray<double>();
    arktest.assertEQ(plainArray.remove(TestData.testNumber1), undefined,
        "The result should be undefined when container is empty");
    arktest.assertEQ(plainArray.length, 0, "The sequence should not change");
}

function testRemoveAtValidIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.removeAt(TestData.testNumber3), TestData.testNumber3, "The result should be removed value");
    arktest.assertEQ(plainArray.toString(), TestData.testString1, "The removed sequence is normal");
}

function testRemoveAtBoundaryIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.removeAt(TestData.testNumber4), TestData.testNumber4, "The result should be removed value");
    arktest.assertEQ(plainArray.removeAt(0), 0, "The result should be removed value");
    arktest.assertEQ(plainArray.toString(), TestData.testString3, "The removed sequence is normal");
}

function testRemoveAtExceededIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.removeAt(-TestData.testNumber1), undefined);
    arktest.assertEQ(plainArray.removeAt(TestData.testNumber6), undefined);
}

function testRemoveAtWithEmpty() {
    let plainArray = new PlainArray<double>();
    arktest.assertEQ(plainArray.removeAt(0), undefined);
}

function testRangeFromValidRange() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.removeRangeFrom(TestData.testNumber2, TestData.testNumber2), TestData.testNumber2,
        "The result should be 2");
    arktest.assertEQ(plainArray.toString(), TestData.testString4, "The removed sequence is normal");
}

function testRangeFromExceededRange() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.removeRangeFrom(TestData.testNumber2, TestData.testNumber6), TestData.testNumber3,
        "The result should be 3");
    arktest.assertEQ(plainArray.toString(), TestData.testString5, "The removed sequence is normal");
}

function testRangeFromExceededIndex() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: -2" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    let exceptionCheckTwo = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: 6" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {plainArray.removeRangeFrom(-TestData.testNumber2, TestData.testNumber2)}, exceptionCheck);
    arktest.expectThrow(() => {plainArray.removeRangeFrom(TestData.testNumber6, TestData.testNumber2)}, exceptionCheckTwo);
}

function testRangeFromWithEmpty() {
    let plainArray = new PlainArray<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }

    arktest.expectThrow(() => {plainArray.removeRangeFrom(0, TestData.testNumber2)}, exceptionCheck);
}

function testRangeFromWithDouble() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    arktest.assertEQ(plainArray.removeRangeFrom(TestData.testNumber2, Math.floor(TestData.testDouble).toInt()), TestData.testNumber3,
        "The result should be 3");
    arktest.assertEQ(plainArray.toString(), TestData.testString5, "The removed sequence is normal");
}

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

function main(): int {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("PlainArray create a single-type container", testCreateSingleTypeInstance);
    suite.addTest("PlainArray create a composite-type container", testCreateCompositeTypeContainer);
    suite.addTest("PlainArray length syncs with element count changes for single-type", testLengthSyncSingleType);
    suite.addTest("PlainArray length syncs with element count changes for multi-type", testLengthSyncMultiType);
    suite.addTest("PlainArray length with add large value", testLengthWithLargeValue);
    suite.addTest("PlainArray isEmpty is false", testIsEmptyAfterAddingElement);
    suite.addTest("PlainArray isEmpty is true", testIsEmptyInitially);
    suite.addTest("PlainArray has", testHasWithExisting);
    suite.addTest("PlainArray has with inexistent key", testHasWithNonExisting);
    return suite.run();
}

function initializePlainArray<T>(plainArray: PlainArray<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        plainArray.add(i, sourceArray[i]);
    }
}

class Temp {
    age: double = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber1w: int = 10000;
    static readonly testString1: string = "squirrel";
    static readonly testObject1: Temp = { age: 10, name: "li" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
}

function testCreateSingleTypeInstance() {
    let plainArray: PlainArray<double>;
    arktest.expectNoThrow(() => {plainArray = new PlainArray<double>()});
}

function testCreateCompositeTypeContainer() {
    let plainArray: PlainArray<double | string | boolean | object | null | undefined>;
    arktest.expectNoThrow(() => {plainArray = new PlainArray<double | string | boolean | object | null | undefined>()});
}

function testLengthSyncSingleType() {
    let plainArray = new PlainArray<double>();
    arktest.assertEQ(plainArray.length, 0, "PlainArray length should be 0");
    initializePlainArray(plainArray, TestData.forNumber1);
    arktest.assertEQ(plainArray.length, TestData.forNumber1.length,
        "PlainArray length should be TestData.forNumber1 length");
    plainArray.remove(TestData.testNumber1);
    arktest.assertEQ(plainArray.length, TestData.forNumber1.length - TestData.testNumber1, "PlainArray length should - 1");
}

function testLengthSyncMultiType() {
    let plainArray = new PlainArray<double | string | boolean | null | object | undefined>();
    arktest.assertEQ(plainArray.length, 0, "PlainArray length should be 0");
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber2, TestData.testString1);
    plainArray.add(TestData.testNumber3, false);
    plainArray.add(TestData.testNumber4, TestData.testObject1);
    plainArray.add(TestData.testNumber5, null);
    plainArray.add(TestData.testNumber6, undefined);
    arktest.assertEQ(plainArray.length, TestData.testNumber6, "PlainArray length should be 6");
    plainArray.remove(TestData.testNumber1);
    plainArray.remove(TestData.testNumber4);
    arktest.assertEQ(plainArray.length, TestData.testNumber4, "PlainArray length should be 4");
}

function testLengthWithLargeValue() {
    let plainArray = new PlainArray<double>();
    for (let i: int = 0; i < TestData.testNumber1w; i++) {
        plainArray.add(i, TestData.testNumber1);
    }
    arktest.assertEQ(plainArray.length, TestData.testNumber1w, "PlainArray length should be 10000 after adding elements");
}

function testIsEmptyAfterAddingElement() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber1);
    arktest.assertFalse(plainArray.isEmpty(), "The plainArray should not be empty");
    arktest.assertEQ(plainArray.length, TestData.forNumber1.length,
        "The plainArray length should be TestData.forNumber1 length");
}

function testIsEmptyInitially() {
    let plainArray = new PlainArray<double>();
    arktest.assertTrue(plainArray.isEmpty(), "The plainArray should be empty");
    arktest.assertEQ(plainArray.length, 0, "The plainArray length should be 0");
}

function testHasWithExisting() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber1);
    for (let i: int = 0; i < plainArray.length; i++) {
        arktest.assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        arktest.assertEQ(plainArray.getValueAt(i), TestData.forNumber1[i], "The value should be exist");
    }
    arktest.assertTrue(plainArray.has(TestData.testNumber1), "The plainArray should contain the key");
}

function testHasWithNonExisting() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber1);
    for (let i: int = 0; i < plainArray.length; i++) {
        arktest.assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        arktest.assertEQ(plainArray.getValueAt(i), TestData.forNumber1[i], "The value should be exist");
    }
    arktest.assertFalse(plainArray.has(TestData.testNumber1w), "The plainArray should not contain the key");
}

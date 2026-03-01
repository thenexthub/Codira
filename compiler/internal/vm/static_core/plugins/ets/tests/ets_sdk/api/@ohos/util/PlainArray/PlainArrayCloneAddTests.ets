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

    suite.addTest("PlainArray clone non-empty container", testCloneWithNonEmpty);
    suite.addTest("PlainArray clone empty container", testCloneWithEmpty);
    suite.addTest("PlainArray clone with changing clone container", testCloneWithChanging);
    suite.addTest("PlainArray add to single-type container", testAddSingleType);
    suite.addTest("PlainArray add to composite-type container", testAddCompositeType);
    suite.addTest("PlainArray add same key", testAddSameKey);
    suite.addTest("PlainArray add same value", testAddSameValue);
    suite.addTest("PlainArray add special double", testAddSpecialNumber);

    return suite.run();
}

function initializePlainArray<T>(plainArray: PlainArray<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        plainArray.add(i, sourceArray[i]);
    }
}

class Tmp {
  age: double = 0;

  constructor(age: double) {
    this.age = age;
  }
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumberMax: int = 2147483647;
    static readonly testNumberMin: int = -2147483648;
    static readonly testString1: string = "squirrel";
    static readonly testString2: string = "1:1,2:squirrel,3:false,4:[object Object],5:null,6:undefined";
    static readonly testString3: string = "1:1,2:1";
    static readonly testObject1 = new Tmp(10);
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testCloneWithNonEmpty() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let clonePlainArray = plainArray.clone();
    arktest.assertEQ(plainArray.toString(), clonePlainArray.toString(), "The elements of both containers should be same");
}

function testCloneWithEmpty() {
    let plainArray = new PlainArray<double>();
    let clonePlainArray = plainArray.clone();
    arktest.assertEQ(plainArray.length, clonePlainArray.length, "The length should be same as original plainArray");
    arktest.assertEQ(clonePlainArray.length, 0, "The length should be 0");
}

function testCloneWithChanging() {
    let plainArray = new PlainArray<double>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let clonePlainArray = plainArray.clone();
    arktest.assertEQ(plainArray.toString(), clonePlainArray.toString(), "The elements of both containers should be same");
    clonePlainArray.add(TestData.testNumber6, TestData.testNumber1);
    arktest.assertEQ(clonePlainArray.getKeyAt(TestData.testNumber5), TestData.testNumber6, "The key should be added");
    arktest.assertEQ(clonePlainArray.getValueAt(TestData.testNumber5), TestData.testNumber1, "The value should be added");
    arktest.assertEQ(plainArray.length, TestData.forNumber2.length, "The original plainArray length should not change");
}

function testAddSingleType() {
    let plainArray = new PlainArray<double>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    arktest.assertEQ(plainArray.length, TestData.testNumber1, "PlainArray length should be 1");
}

function testAddCompositeType() {
    let plainArray = new PlainArray<double | string | boolean | object | null | undefined>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber2, TestData.testString1);
    plainArray.add(TestData.testNumber3, false);
    plainArray.add(TestData.testNumber4, TestData.testObject1);
    plainArray.add(TestData.testNumber5, null);
    plainArray.add(TestData.testNumber6, undefined);
    console.log(plainArray.toString());
    arktest.assertEQ(plainArray.length, TestData.testNumber6, "PlainArray length should be 6");
    arktest.assertEQ(plainArray.toString(), TestData.testString2, "The elements should be added");
}

function testAddSameKey() {
    let plainArray = new PlainArray<double>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber1, TestData.testNumber2);
    arktest.assertEQ(plainArray.length, TestData.testNumber1, "PlainArray length should be 1");
    arktest.assertEQ(plainArray.get(TestData.testNumber1), TestData.testNumber2,
        "The value corresponding to key 1 should be 2");
}

function testAddSameValue() {
    let plainArray = new PlainArray<double>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber2, TestData.testNumber1);
    arktest.assertEQ(plainArray.length, TestData.testNumber2, "PlainArray length should be 2");
    arktest.assertEQ(plainArray.toString(), TestData.testString3, "The elements should be added");
}

function testAddSpecialNumber() {
    let plainArray = new PlainArray<double>();
    plainArray.add(-TestData.testNumber1, TestData.testNumber1);
    plainArray.add(0, TestData.testNumber1);
    plainArray.add(TestData.testNumberMax, TestData.testNumber1);
    plainArray.add(TestData.testNumberMin, TestData.testNumber1);
    arktest.assertTrue(plainArray.has(-TestData.testNumber1), "The key should be added");
    arktest.assertTrue(plainArray.has(0), "The key should be added");
    arktest.assertTrue(plainArray.has(TestData.testNumberMax), "The key should be added");
    arktest.assertTrue(plainArray.has(TestData.testNumberMin), "The key should be added");
}

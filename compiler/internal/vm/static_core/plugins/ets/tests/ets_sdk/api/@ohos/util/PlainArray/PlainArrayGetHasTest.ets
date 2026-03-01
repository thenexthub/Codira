/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function main() {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("Testing PlainArray constructor with empty array", testConstructorWithEmptyArray);
    suite.addTest("Testing PlainArray isEmpty with empty array", testIsEmptyWithEmptyArray);
    suite.addTest("Testing PlainArray isEmpty after add and remove operations", testIsEmptyAfterAddAndRemove);
    suite.addTest("Testing PlainArray isEmpty after add and clear operations", testIsEmptyAfterAddAndClear);
    suite.addTest("Testing PlainArray has with existing key", testHasWithExistingKey);
    suite.addTest("Testing PlainArray has with empty string value", testHasWithEmptyStringValue);
    suite.addTest("Testing PlainArray has with boundary keys", testHasWithBoundaryKeys);
    suite.addTest("Testing PlainArray get with existing key", testGetWithExistingKey);
    suite.addTest("Testing PlainArray get with empty string value", testGetWithEmptyStringValue);
    suite.addTest("Testing PlainArray get with boundary keys", testGetWithBoundaryKeys);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CONSTRUCTOR_0100
function testConstructorWithEmptyArray() {
    let plainArray = new PlainArray<string>();
    arktest.assertEQ(plainArray.length, 0);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ISEMPTY_0100
function testIsEmptyWithEmptyArray() {
    let plainArray = new PlainArray<string>();
    arktest.assertTrue(plainArray.isEmpty(), "plainArray.isEmpty() should return true");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ISEMPTY_0200
function testIsEmptyAfterAddAndRemove() {
    let plainArray = new PlainArray<string>();
    plainArray.add(0, "zhang san");
    arktest.assertFalse(plainArray.isEmpty(), "plainArray.isEmpty() should return false");
    plainArray.remove(0);
    arktest.assertTrue(plainArray.isEmpty(), "plainArray.isEmpty() should return true");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ISEMPTY_0300
function testIsEmptyAfterAddAndClear() {
    let plainArray = new PlainArray<string>();
    plainArray.add(0, "zhang san");
    plainArray.clear();
    arktest.assertTrue(plainArray.isEmpty(), "plainArray.isEmpty() should return true");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_HAS_0100
function testHasWithExistingKey() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    let result = plainArray.has(1);
    arktest.assertTrue(result, "plainArray.has(1) should return true for an existing key");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_HAS_0200
function testHasWithEmptyStringValue() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "");
    let result = plainArray.has(1);
    arktest.assertTrue(result, "plainArray.has(1) should return true even if the value is an empty string");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_HAS_0500
function testHasWithBoundaryKeys() {
    let plainArray = new PlainArray<string>();
    plainArray.add(0, "zhang san");
    arktest.assertTrue(plainArray.has(0), "plainArray.has(0) should return true");
    plainArray.add(-1, "zhang san");
    arktest.assertTrue(plainArray.has(-1), "plainArray.has(-1) should return true");
    let num: int = 2147483647;
    plainArray.add(num, "li si");
    arktest.assertTrue(plainArray.has(num), "plainArray.has(num) should return true");
    let num1: int = -2147483648;
    plainArray.add(num1, "li si");
    arktest.assertTrue(plainArray.has(num1), "plainArray.has(num1) should return true");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GET_0100
function testGetWithExistingKey() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.get(1);
    arktest.assertEQ(result, 'squirrel');
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GET_0200
function testGetWithEmptyStringValue() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "");
    plainArray.add(2, "sparrow");
    let result = plainArray.get(1);
    arktest.assertEQ(result, '');
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GET_0500
function testGetWithBoundaryKeys() {
    let plainArray = new PlainArray<string>();
    plainArray.add(0, "zhang san");
    arktest.assertEQ(plainArray.get(0), "zhang san");
    plainArray.add(-1, "zhang san");
    arktest.assertEQ(plainArray.get(-1), "zhang san");
    let num: int = 2147483647;
    plainArray.add(num, "li si");
    arktest.assertEQ(plainArray.get(num), "li si");
    let num1: int = -2147483648;
    plainArray.add(num1, "li si");
    arktest.assertEQ(plainArray.get(num1), "li si");
}

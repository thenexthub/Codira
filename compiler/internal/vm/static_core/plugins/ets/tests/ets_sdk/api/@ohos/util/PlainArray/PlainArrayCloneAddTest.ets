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

class Tmp {
    age: double = 0;

    constructor(age: double) {
        this.age = age;
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("Testing PlainArray clone with string values", testCloneWithStringValues);
    suite.addTest("Testing PlainArray clone with double values", testCloneWithNumberValues);
    suite.addTest("Testing PlainArray clone with object values", testCloneWithObjectValues);
    suite.addTest("Testing PlainArray clone and add operation", testCloneAndAddOperation);
    suite.addTest("Testing PlainArray clone with large string values", testCloneWithLargeStringValues);
    suite.addTest("Testing PlainArray add operation with large string", testAddOperationWithLargeString);
    suite.addTest("Testing PlainArray add operation with string values", testAddOperationWithStringValues);
    suite.addTest("Testing PlainArray remove operation with existing key", testRemoveOperationWithExistingKey);
    suite.addTest("Testing PlainArray remove operation with non-existing key", testRemoveOperationWithNonExistingKey);
    suite.addTest("Testing PlainArray remove operation with boundary keys", testRemoveOperationWithBoundaryKeys);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLONE_0100
function testCloneWithStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let newPlainArray = plainArray.clone();
    arktest.assertEQ(newPlainArray.length, plainArray.length);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLONE_0200
function testCloneWithNumberValues() {
    let plainArray = new PlainArray<double>();
    plainArray.add(1, 1);
    plainArray.add(2, 2);
    let newPlainArray = plainArray.clone();
    arktest.assertEQ(newPlainArray.length, plainArray.length);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLONE_0300
function testCloneWithObjectValues() {
    let plainArray = new PlainArray<Tmp>();
    plainArray.add(1, new Tmp(10));
    plainArray.add(2, new Tmp(12));
    let newPlainArray = plainArray.clone();
    arktest.assertEQ(newPlainArray.length, plainArray.length);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLONE_0400
function testCloneAndAddOperation() {
    let plainArray = new PlainArray<Tmp>();
    plainArray.add(1, new Tmp(10));
    plainArray.add(2, new Tmp(12));
    let newPlainArray = plainArray.clone();
    newPlainArray.add(3, new Tmp(13));
    arktest.assertEQ(plainArray.length, 2);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLONE_0500
function testCloneWithLargeStringValues() {
    let plainArray = new PlainArray<string>();
    let param = "a".repeat(5120);
    plainArray.add(1, param);
    let newPlainArray = plainArray.clone();
    arktest.assertEQ(plainArray.length, newPlainArray.length);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ADD_0100
function testAddOperationWithLargeString() {
    let plainArray = new PlainArray<string>();
    let param = "a".repeat(5120);
    plainArray.add(1, param);
    arktest.assertEQ(plainArray.length, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ADD_0200
function testAddOperationWithStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    arktest.assertEQ(plainArray.length, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVE_0100
function testRemoveOperationWithExistingKey() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.remove(2);
    arktest.assertEQ(result, "sparrow");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVE_0400
function testRemoveOperationWithNonExistingKey() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "zhang san");
    plainArray.add(2, "zhang san1");
    let result = plainArray.remove(3);
    arktest.assertEQ(result, undefined);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVE_0500
function testRemoveOperationWithBoundaryKeys() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "zhang san");
    plainArray.add(2, "zhang san1");
    let num: int = 2147483647;
    plainArray.add(num, "li si");
    arktest.assertEQ(plainArray.remove(num), "li si");
    let num1: int = -2147483648;
    plainArray.add(num1, "li si");
    arktest.assertEQ(plainArray.remove(num1), "li si");
}

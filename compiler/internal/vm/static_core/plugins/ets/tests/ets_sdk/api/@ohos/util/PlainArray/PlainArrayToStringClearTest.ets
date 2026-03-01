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

    suite.addTest("Testing PlainArray toString with string and double values", testToStringWithStringAndNumberValues);
    suite.addTest("Testing PlainArray toString with empty array", testToStringWithEmptyArray);
    suite.addTest("Testing PlainArray toString with duplicate string values", testToStringWithDuplicateStringValues);
    suite.addTest("Testing PlainArray toString with mixed types (string, double, boolean)", testToStringWithMixedTypes);
    suite.addTest("Testing PlainArray toString with large array", testToStringWithLargeArray);
    suite.addTest("Testing PlainArray clear with multiple elements", testClearWithMultipleElements);
    suite.addTest("Testing PlainArray clear and re-add elements", testClearAndReAddElements);
    suite.addTest("Testing PlainArray clear with empty array", testClearWithEmptyArray);
    suite.addTest("Testing PlainArray clear with string array", testClearWithStringArray);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_TOSTRING_0100
function testToStringWithStringAndNumberValues() {
    let plainArray = new PlainArray<string | double>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.toString();
    arktest.assertEQ(result, '1:squirrel,2:sparrow');
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_TOSTRING_0300
function testToStringWithEmptyArray() {
    let plainArray = new PlainArray<string | double>();
    let result = plainArray.toString();
    arktest.assertEQ(result, '');
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_TOSTRING_0400
function testToStringWithDuplicateStringValues() {
    let plainArray = new PlainArray<string | double>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.add(3, "sparrow");
    let result = plainArray.toString();
    arktest.assertEQ(result, '1:squirrel,2:sparrow,3:sparrow');
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_TOSTRING_0500
function testToStringWithMixedTypes() {
    let plainArray = new PlainArray<string | double | boolean>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, 10);
    plainArray.add(3, true);
    let result = plainArray.toString();
    arktest.assertEQ(result, '1:squirrel,2:10,3:true');
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_TOSTRING_0600
function testToStringWithLargeArray() {
    let plainArray = new PlainArray<string | double | boolean>();
    for (let i: int = 0; i < 1024; i++) {
        plainArray.add(i, i);
    }
    plainArray.toString();
    arktest.assertEQ(plainArray.length, 1024);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLEAR_0100
function testClearWithMultipleElements() {
    let plainArray = new PlainArray<string | double | boolean>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.clear();
    arktest.assertEQ(plainArray.length, 0);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLEAR_0200
function testClearAndReAddElements() {
    let plainArray = new PlainArray<string | double | boolean>();
    plainArray.add(1, "squirrel");
    plainArray.clear();
    plainArray.add(2, "sparrow");
    arktest.assertEQ(plainArray.length, 1);
    arktest.assertEQ(plainArray.get(2), "sparrow");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLEAR_0300
function testClearWithEmptyArray() {
    let plainArray = new PlainArray<string | double | boolean>();
    plainArray.clear();
    arktest.assertEQ(plainArray.length, 0);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_CLEAR_0400
function testClearWithStringArray() {
    let plainArray = new PlainArray<string>();
    plainArray.clear();
    arktest.assertEQ(plainArray.length, 0);
}

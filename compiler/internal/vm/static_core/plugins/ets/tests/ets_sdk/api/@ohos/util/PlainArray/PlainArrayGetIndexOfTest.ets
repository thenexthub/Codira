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

    suite.addTest("Testing PlainArray getIndexOfKey with empty string value", testGetIndexOfKeyWithEmptyStringValue);
    suite.addTest("Testing PlainArray getIndexOfKey with object values", testGetIndexOfKeyWithObjectValues);
    suite.addTest("Testing PlainArray getIndexOfKey with string values", testGetIndexOfKeyWithStringValues);
    suite.addTest("Testing PlainArray getIndexOfValue with empty string value",
        testGetIndexOfValueWithEmptyStringValue);
    suite.addTest("Testing PlainArray getIndexOfValue with object values", testGetIndexOfValueWithObjectValues);
    suite.addTest("Testing PlainArray getIndexOfValue with string values", testGetIndexOfValueWithStringValues);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETINDEXOFKEY_0100
function testGetIndexOfKeyWithEmptyStringValue() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "");
    let result = plainArray.getIndexOfKey(2);
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETINDEXOFKEY_0200
function testGetIndexOfKeyWithObjectValues() {
    let plainArray = new PlainArray<Tmp>();
    plainArray.add(1, new Tmp(10));
    plainArray.add(2, new Tmp(12));
    let result = plainArray.getIndexOfKey(2);
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETINDEXOFKEY_0400
function testGetIndexOfKeyWithStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getIndexOfKey(2);
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETINDEXOVALUE_0100
function testGetIndexOfValueWithEmptyStringValue() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "");
    let result = plainArray.getIndexOfValue("");
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETINDEXOVALUE_0200
function testGetIndexOfValueWithObjectValues() {
    let plainArray = new PlainArray<Tmp>();
    let param1 = new Tmp(10);
    plainArray.add(1, param1);
    let param2 = new Tmp(10);
    plainArray.add(2, param2);
    let result = plainArray.getIndexOfValue(param2);
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETINDEXOVALUE_0400
function testGetIndexOfValueWithStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getIndexOfValue("sparrow");
    arktest.assertEQ(result, 1);
}

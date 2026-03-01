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
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("Testing PlainArray getKeyAt with out-of-range index", testGetKeyAtOutOfRangeIndex);
    suite.addTest("Testing PlainArray getKeyAt with valid index", testGetKeyAtValidIndex);
    suite.addTest("Testing PlainArray getKeyAt and getIndexOfKey consistency", testGetKeyAtAndIndexOfKeyConsistency);
    suite.addTest("Testing PlainArray getValueAt with out-of-range index", testGetValueAtOutOfRangeIndex);
    suite.addTest("Testing PlainArray getValueAt with valid index", testGetValueAtValidIndex);
    suite.addTest("Testing PlainArray getValueAt and getIndexOfValue consistency",
        testGetValueAtAndIndexOfValueConsistency);
    suite.addTest("Testing PlainArray setValueAt with valid index and mixed types", testSetValueAtValidIndexMixedTypes);
    suite.addTest("Testing PlainArray setValueAt with out-of-range index", testSetValueAtOutOfRangeIndex);
    suite.addTest("Testing PlainArray setValueAt with boundary values and large strings", testSetValueAtBoundaryValues);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETKEYAT_0100
function testGetKeyAtOutOfRangeIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getKeyAt(3);
    arktest.assertEQ(result, -1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETKEYAT_0200
function testGetKeyAtValidIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getKeyAt(1);
    arktest.assertEQ(result, 2);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETKEYAT_0500
function testGetKeyAtAndIndexOfKeyConsistency() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getKeyAt(1);
    let index = plainArray.getIndexOfKey(result!);
    arktest.assertEQ(index, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETVALUEAT_0100
function testGetValueAtOutOfRangeIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == 
                `The value of \"index\" is out of range. It must be >= 0 && <= 1. Received value is: 2` &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {plainArray.getValueAt(2)}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETVALUEAT_0200
function testGetValueAtValidIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getValueAt(1);
    arktest.assertEQ(result, "sparrow");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_GETVALUEAT_0500
function testGetValueAtAndIndexOfValueConsistency() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.getValueAt(1);
    let index = plainArray.getIndexOfValue(result);
    arktest.assertEQ(index, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_SETVALUEAT_0100
function testSetValueAtValidIndexMixedTypes() {
    let plainArray = new PlainArray<string | double>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.setValueAt(1, 3546);
    arktest.assertEQ(plainArray.getValueAt(1), 3546);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_SETVALUEAT_0200
function testSetValueAtOutOfRangeIndex() {
    let plainArray = new PlainArray<string | double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`
        }
        return false;
    }
    arktest.expectThrow(() => {plainArray.setValueAt(0, 3546)}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_SETVALUEAT_0300
function testSetValueAtBoundaryValues() {
    let plainArray = new PlainArray<string | double>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.add(3, "sparrow");
    plainArray.setValueAt(0, 2147483647);
    plainArray.setValueAt(1, -2147483648);
    let param = "a".repeat(5120);
    plainArray.setValueAt(2, param);
    arktest.assertEQ(plainArray.length, 3);
}

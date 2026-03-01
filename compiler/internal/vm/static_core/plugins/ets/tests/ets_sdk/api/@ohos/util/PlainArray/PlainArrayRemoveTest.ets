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

    suite.addTest("Testing PlainArray removeAt with valid index", testRemoveAtWithValidIndex);
    suite.addTest("Testing PlainArray removeAt with negative index", testRemoveAtWithNegativeIndex);
    suite.addTest("Testing PlainArray removeAt with out-of-range index", testRemoveAtWithOutOfRangeIndex);
    suite.addTest("Testing PlainArray removeAt with empty array", testRemoveAtWithEmptyArray);
    suite.addTest("Testing PlainArray removeRangeFrom with size 0", testRemoveRangeFromWithSizeZero);
    suite.addTest("Testing PlainArray removeRangeFrom with valid range", testRemoveRangeFromWithValidRange);
    suite.addTest("Testing PlainArray removeRangeFrom with Math.floor size", testRemoveRangeFromWithMathFloorSize);
    suite.addTest("Testing PlainArray removeRangeFrom with empty array", testRemoveRangeFromWithEmptyArray);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVEAT_0100
function testRemoveAtWithValidIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.removeAt(1);
    arktest.assertEQ(result, "sparrow");
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVEAT_0200
function testRemoveAtWithNegativeIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.removeAt(-1);
    arktest.assertEQ(result, undefined);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVEAT_0300
function testRemoveAtWithOutOfRangeIndex() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.removeAt(2);
    arktest.assertEQ(result, undefined);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVEAT_0400
function testRemoveAtWithEmptyArray() {
    let plainArray = new PlainArray<string>();
    let result = plainArray.removeAt(0);
    arktest.assertEQ(result, undefined);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVERANGEFROM_0100
function testRemoveRangeFromWithSizeZero() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                `The value of \"size\" is out of range. It must be > 0. Received value is: 0` &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {plainArray.removeRangeFrom(1, 0)}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVERANGEFROM_0200
function testRemoveRangeFromWithValidRange() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.removeRangeFrom(1, 3);
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVERANGEFROM_0300
function testRemoveRangeFromWithMathFloorSize() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let result = plainArray.removeRangeFrom(1, Math.floor(7.5).toInt());
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_REMOVERANGEFROM_0400
function testRemoveRangeFromWithEmptyArray() {
    let plainArray = new PlainArray<string>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`
        }
        return false;
    }
    arktest.expectThrow(() => {plainArray.removeRangeFrom(0, 0)}, exceptionCheck);
}

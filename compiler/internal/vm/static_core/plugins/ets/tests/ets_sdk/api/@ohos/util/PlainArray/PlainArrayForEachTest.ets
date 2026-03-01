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

    suite.addTest("Testing PlainArray forEach with string values", testForEachWithStringValues);
    suite.addTest("Testing PlainArray forEach with duplicate string values", testForEachWithDuplicateStringValues);
    suite.addTest("Testing PlainArray forEach with an empty array", testForEachWithEmptyArray);
    suite.addTest("Testing PlainArray forEach with a single element", testForEachWithSingleElement);
    suite.addTest("Testing PlainArray forEach with value and index validation", testForEachWithValueAndIndexValidation);
    suite.addTest("Testing PlainArray Symbol.iterator with string values", testSymbolIteratorWithStringValues);
    suite.addTest("Testing PlainArray Symbol.iterator with duplicate string values",
        testSymbolIteratorWithDuplicateStringValues);
    suite.addTest("Testing PlainArray Symbol.iterator with an empty array", testSymbolIteratorWithEmptyArray);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_FOREACH_0100
function testForEachWithStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.forEach((value: string, index?: int) => {
        arktest.assertEQ(plainArray.get(index!), value);
    });
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_FOREACH_0200
function testForEachWithDuplicateStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.add(3, "sparrow");
    plainArray.forEach((value: string, index?: int) => {
        arktest.assertEQ(plainArray.get(index!), value);
    });
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_FOREACH_0300
function testForEachWithEmptyArray() {
    let plainArray = new PlainArray<string>();
    plainArray.forEach((value: string, index?: int) => {
        arktest.assertEQ(plainArray.get(index!), value);
    });
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_FOREACH_0600
function testForEachWithSingleElement() {
    let plainArray = new PlainArray<string>();
    plainArray.add(0, "0");
    plainArray.forEach((value) => {
        arktest.assertEQ(plainArray.get(0), value);
    });
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_FOREACH_0700
function testForEachWithValueAndIndexValidation() {
    let plainArray = new PlainArray<string>();
    plainArray.add(0, "0");
    plainArray.forEach((value, index, plainArray) => {
        arktest.assertEQ(plainArray.get(0), value);
    });
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ITERATOR_0100
function testSymbolIteratorWithStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    let iter = plainArray.$_iterator();
    let temp: IteratorResult<[Int, string]> = iter.next();
    while (!temp.done) {
        let key = temp.value![0].toDouble();
        let value = temp.value![1] as string;
        arktest.assertEQ(plainArray.get(key.toInt()), value);
        temp = iter.next();
    }
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ITERATOR_0200
function testSymbolIteratorWithDuplicateStringValues() {
    let plainArray = new PlainArray<string>();
    plainArray.add(1, "squirrel");
    plainArray.add(2, "sparrow");
    plainArray.add(3, "sparrow");
    let iter = plainArray.$_iterator();
    let temp: IteratorResult<[Int, string]> = iter.next();
    while (!temp.done) {
        let key = temp.value![0].toDouble();
        let value = temp.value![1] as string;
        arktest.assertEQ(plainArray.get(key.toInt()), value);
        temp = iter.next();
    }
}

// Test cases ported from ArkTS 1.0: PLAINARRAY_ITERATOR_0300
function testSymbolIteratorWithEmptyArray() {
    let plainArray = new PlainArray<string>();
    let iter = plainArray.$_iterator();
    let temp: IteratorResult<[Int, string]> = iter.next();
    while (!temp.done) {
        let key = temp.value![0].toDouble();
        let value = temp.value![1] as string;
        arktest.assertEQ(plainArray.get(key.toInt()), value);
        temp = iter.next();
    }
}

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

import HashMap from "@ohos.util.HashMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("HashMap ForEach Keys Values Iterator API tests")

    suite.addTest("HashMap forEach the data with no reference", testForEachWithoutReference)
    suite.addTest("HashMap forEach the data with reference is value", testForEachWithValueReference)
    suite.addTest("HashMap forEach the data with reference are value and key", testForEachWithKeyAndValueReference)
    suite.addTest("HashMap forEach the data with three references", testForEachWithThreeReferences)
    suite.addTest("HashMap keys return iterator by key", testKeysIterator)
    suite.addTest("HashMap keys return iterator by key with null hashMap", testKeysIteratorWithEmptyHashMap)
    suite.addTest("HashMap values return iterator by value", testValuesIterator)
    suite.addTest("HashMap values return iterator by value with null HashMap", testValuesIteratorWithEmptyHashMap)
    suite.addTest("HashMap $_iterator return iterator", testIterator)
    suite.addTest("HashMap $_iterator return iterator with null hashMap", testIteratorWithEmptyHashMap)

    return suite.run()
}

function testForEachWithoutReference() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    let count: int = 0;
    hashMap.forEach(() => {
        count++;
    });
    arktest.assertEQ(count, 2, "The hashMap forEach should iterate over all elements");
}

function testForEachWithValueReference() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    let flag: boolean = false;
    hashMap.forEach((value: double) => {
        flag = hashMap.hasValue(value);
        arktest.assertEQ(flag, true, "The hashMap forEach should iterate over all values");
    });
}

function testForEachWithKeyAndValueReference() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    let flag1: boolean = false;
    let flag2: boolean = false;
    hashMap.forEach((value: double, key: string) => {
        flag1 = hashMap.hasValue(value);
        flag2 = hashMap.hasKey(key);
        arktest.assertEQ(flag1, true, "The hashMap forEach should iterate over all values");
        arktest.assertEQ(flag2, true, "The hashMap forEach should iterate over all keys");
    });
}

function testForEachWithThreeReferences() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    let flag1: boolean = false;
    let flag2: boolean = false;
    hashMap.forEach((value: double, key: string, tempHashMap: HashMap<string, double>) => {
        flag1 = tempHashMap.hasValue(value);
        flag2 = tempHashMap.hasKey(key);
        arktest.assertEQ(flag1, true, "The hashMap forEach should iterate over all values");
        arktest.assertEQ(flag2, true, "The hashMap forEach should iterate over all keys");
    });
}

function testKeysIterator() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    let iteratorSetValues = hashMap.keys();
    arktest.assertEQ(iteratorSetValues.next().value, "sparrow", "The hashMap keys iterator should return the first key");
    arktest.assertEQ(iteratorSetValues.next().value, "gull", "The hashMap keys iterator should return the second key");
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashMap keys iterator.done should true for no more keys");
}

function testKeysIteratorWithEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let iteratorSetValues = hashMap.keys();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashMap keys iterator.done should true when the hashMap is empty");
}

function testValuesIterator() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    let iteratorSetValues = hashMap.values();
    arktest.assertEQ(iteratorSetValues.next().value, 123, "The hashMap values iterator should return the first value");
    arktest.assertEQ(iteratorSetValues.next().value, 357, "The hashMap values iterator should return the second value");
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashMap values iterator.done should true for no more values");
}

function testValuesIteratorWithEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let iteratorSetValues = hashMap.values();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashMap values iterator.done should true when the hashMap is empty");
}

function testIterator() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 123);
    hashMap.set("gull", 357);
    hashMap.set("test", 111);
    hashMap.set("apply", 222);
    let iterator = hashMap.$_iterator();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "sparrow,123,gull,357,test,111,apply,222";
    arktest.assertEQ(str, strAll, "The hashMap $_iterator should return all key-value pairs");
}

function testIteratorWithEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let iteratorSetValues = hashMap.$_iterator();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashMap iterator.done should true when the hashMap is empty");
}

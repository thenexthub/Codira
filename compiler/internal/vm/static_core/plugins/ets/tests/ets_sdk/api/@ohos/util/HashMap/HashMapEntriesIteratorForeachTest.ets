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
    const suite = new arktest.ArkTestsuite("HashMap Entries Iterator Foreach API tests")

    suite.addTest("Testing HashMap is empty check length", testEntriesEmptyMap)
    suite.addTest("Testing HashMap has check everyOne and allLength", testEntriesPopulatedMapIterate)
    suite.addTest("Testing HashMap has entries check remove", testEntriesPopulatedMapRemoveIter)
    suite.addTest("Testing HashMap has entries check replace", testEntriesPopulatedMapReplaceIter)
    suite.addTest("Testing HashMap has entries if clear check length", testEntriesPopulatedMapClearIter)
    suite.addTest("Testing HashMap has entries if add data check length", testEntriesPopulatedMapAddIter)
    suite.addTest("Testing HashMap is empty while loop check", testSymbolIteratorEmpty)
    suite.addTest("Testing HashMap has data while loop check value and length", testSymbolIteratorPopulated)
    suite.addTest("Testing HashMap is empty foreach check length", testForeachEmptyMap)
    suite.addTest("Testing HashMap has foreach check value and allLength", testForeachPopulatedMap)
    suite.addTest("Testing HashMap has foreach check remove", testForeachPopulatedMapRemoveCb)
    suite.addTest("Testing HashMap has foreach check replace", testForeachPopulatedMapReplaceCb)
    suite.addTest("Testing HashMap has foreach if clear check length", testForeachPopulatedMapClearCb)
    suite.addTest("Testing HashMap has foreach if no params check length", testForeachCbNoParams)
    suite.addTest("Testing HashMap has foreach if one param check length", testForeachCbOneParam)
    suite.addTest("Testing HashMap has foreach ArgThis", testForeachWithThis)
    suite.addTest("Testing HashMap has foreach ArgThis again", testForeachCbThreeParams)
    suite.addTest("Testing HashMap has data while loop check value and length", testCustomTypeOps)
    suite.addTest("Testing HashMap has data call different apis to check length changed", testCustomLengthChanges)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHMAP_ENTRIES_0100
function testEntriesEmptyMap() {
    const hashMap = new HashMap<double, double>();
    let entries = hashMap.entries();
    let temp = entries.next();
    arktest.assertTrue(temp.done, "Expected iterator to be done for an empty set");
    while (!temp.done) {
        arktest.assertTrue(false, "Loop body should not execute for an empty set iterator");
        temp = entries.next();
    }
}

//Test cases ported from ArkTS 1.0:HASHMAP_ENTRIES_0200
function testEntriesPopulatedMapIterate() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let entries = hashMap.entries();
    let temp = entries.next();
    let index = 0;
    while (!temp.done) {
        arktest.assertEQ(temp.value![0], index);
        arktest.assertEQ(temp.value![1], index);
        index++;
        temp = entries.next();
    }
    arktest.assertEQ(hashMap.length, 3);
}
//Test cases ported from ArkTS 1.0:HASHMAP_ENTRIES_0300
function testEntriesPopulatedMapRemoveIter() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let entries = hashMap.entries();
    let temp = entries.next();
    while (!temp.done) {
        arktest.assertEQ(hashMap.remove(temp.value![0]), temp.value![1]);
        temp = entries.next();
    }
}

//Test cases ported from ArkTS 1.0:HASHMAP_ENTRIES_0400
function testEntriesPopulatedMapReplaceIter() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let entries = hashMap.entries();
    let temp = entries.next();
    while (!temp.done) {
        let key = temp.value![0];
        let value = temp.value![1];
        arktest.assertTrue(hashMap.replace(key, value ? value + 1 : 0),
            "Expected replace(key, value ? value + 1 : 0) to return true during entries iteration");
        temp = entries.next();
    }
}

//Test cases ported from ArkTS 1.0:HASHMAP_ENTRIES_0500
function testEntriesPopulatedMapClearIter() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let entries = hashMap.entries();
    let temp = entries.next();
    while (!temp.done) {
        hashMap.clear();
        arktest.assertEQ(hashMap.length, 0);
        temp = entries.next();
    }
}

function testEntriesPopulatedMapAddIter() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let entries = hashMap.entries();
    let temp = entries.next();
    let added = false;

    while (!temp.done) {
        if (!added) {
            hashMap.set(3, 3);
            hashMap.set(4, 4);
            added = true;
        }
        temp = entries.next();
    }
    arktest.assertEQ(hashMap.get(3), 3);
    arktest.assertEQ(hashMap.get(4), 4);
    arktest.assertEQ(hashMap.length, 5);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SYMBOL_ITERATOR_0100
function testSymbolIteratorEmpty() {
    const hashMap = new HashMap<double, double>();
    let iter = hashMap.$_iterator();
    let temp = iter.next();
    arktest.assertTrue(temp.done, "Expected iterator to be done for an empty set");
    while (!temp.done) {
        arktest.assertTrue(false, "Loop body should not execute for an empty set iterator");
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:HASHMAP_SYMBOL_ITERATOR_0200
function testSymbolIteratorPopulated() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let iter = hashMap.$_iterator();
    let temp = iter.next();
    let index = 0;
    while (!temp.done) {
        arktest.assertEQ(temp.value![0], index);
        arktest.assertEQ(temp.value![1], index);
        index++;
        temp = iter.next()
    }
    arktest.assertEQ(hashMap.length, 3);
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0100
function testForeachEmptyMap() {
    const hashMap = new HashMap<double, double>();
    hashMap.forEach((value?: double, key?: double) => {
        arktest.assertEQ(hashMap.length, 0);
    });
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0200
function testForeachPopulatedMap() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    let index = 0;
    hashMap.forEach((value?: double, key?: double) => {
        arktest.assertEQ(key, index);
        arktest.assertEQ(value, index);
        index++;
    });
    arktest.assertEQ(hashMap.length, 3);
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0300
function testForeachPopulatedMapRemoveCb() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    hashMap.forEach((value: double, key: double) => {
        arktest.assertEQ(hashMap.remove(key), value);
    });
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0400
function testForeachPopulatedMapReplaceCb() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    hashMap.forEach((value: double, key: double) => {
        arktest.assertTrue(hashMap.replace(key, value ? value + 1 : 0),
            "Expected replace(key, value ? value + 1 : 0) to return true during forEach");
    });
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0500
function testForeachPopulatedMapClearCb() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    hashMap.forEach((value?: double, key?: double) => {
        hashMap.clear();
        arktest.assertEQ(hashMap.length, 0);
    });
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0600
function testForeachCbNoParams() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    hashMap.forEach(() => {
        arktest.assertEQ(hashMap.length, 3);
    });
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0700
function testForeachCbOneParam() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.forEach((value) => {
        arktest.assertEQ(hashMap.length, 1);
        arktest.assertEQ(value, 0);
    });
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0800
function testForeachWithThis() {
    const hashMap = new HashMap<double, double>();
    const hashMapNew = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMapNew.set(0, 0);
    hashMap.forEach((value) => {
        arktest.assertEQ(hashMap.length, 1);
        arktest.assertEQ(value, 0);
    });
    arktest.assertEQ(hashMapNew.length, 1);
    arktest.assertEQ(hashMapNew.get(0), 0);
}

//Test cases ported from ArkTS 1.0:HASHMAP_FOREACH_0900
function testForeachCbThreeParams() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 0);
    hashMap.forEach((value, index, hashMap: HashMap<double, double>) => {
        arktest.assertEQ(hashMap.length, 1);
        arktest.assertEQ(value, 0);
    });
}

class Temp {
    age: number = 0
    name: string = ""
}
//Test cases ported from ArkTS 1.0:HASHMAP_CUSTOM_0100
function testCustomTypeOps() {
    const hashMap = new HashMap<double, Temp>();
    let temp: Temp = {
        age: 18, name: "zhang san"
    };
    hashMap.set(0, temp);
    let temp1: Temp = {
        age: 19, name: "li si"
    };
    hashMap.set(1, temp1);
    let temp2: Temp = {
        age: 31, name: "wang wu"
    };
    hashMap.set(2, temp2);
    const newHashMap = new HashMap<double, Temp>();
    let temp3: Temp = {
        age: 32, name: "zhao liu"
    };
    newHashMap.set(3, temp3);
    hashMap.setAll(newHashMap);
    arktest.assertFalse(hashMap.isEmpty(), "hashMap should not be empty after setAll");
    arktest.assertTrue(hashMap.hasKey(0), "hashMap should have key 0 after setAll");
    arktest.assertTrue(hashMap.hasValue(temp), "hashMap should have value 'temp' after setAll");
    arktest.assertEQ(hashMap.get(0), temp);
    let temp4: Temp = {
        age: 33, name: "wang ma zi"
    };
    arktest.assertTrue(hashMap.replace(1, temp4), "hashMap.replace(1, temp4) should return true");
    arktest.assertEQ(hashMap.remove(0), temp);
    hashMap.clear();
    arktest.assertEQ(hashMap.length, 0);
}

function testCustomLengthChanges() {
    const hashMap = new HashMap<double, double>();
    arktest.assertEQ(hashMap.length, 0);
    hashMap.set(0, 0);
    hashMap.set(1, 1);
    hashMap.set(2, 2);
    arktest.assertEQ(hashMap.length, 3);

    hashMap.set(2, 3);
    arktest.assertEQ(hashMap.length, 3);

    const newHashMap = new HashMap<double, double>();
    arktest.assertEQ(newHashMap.length, 0);

    newHashMap.setAll(hashMap);
    arktest.assertEQ(newHashMap.length, 3);

    hashMap.remove(1);
    arktest.assertEQ(hashMap.length, 2);

    hashMap.clear();
    arktest.assertEQ(hashMap.length, 0);
}

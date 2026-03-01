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
    const suite = new arktest.ArkTestsuite("HashMap Clear Keys Values Replace API tests")

    suite.addTest("Testing HashMap is empty clear and check length get hasKey", testClearEmptyMap)
    suite.addTest("Testing HashMap has data clear and check length get hasKey", testClearPopulatedMap)
    suite.addTest("Testing HashMap has data clear and set same data check length", testClearSetSameKey)
    suite.addTest("Testing HashMap has data clear and set different data check length", testClearSetNewKey)
    suite.addTest("Testing HashMap is empty check keys", testKeysEmpty)
    suite.addTest("Testing HashMap has data check everyOne key", testKeysPopulated)
    suite.addTest("Testing HashMap is empty check keys and set data check keys again", testKeysEmptyThenPopulated)
    suite.addTest("Testing HashMap is empty check values", testValuesEmpty)
    suite.addTest("Testing HashMap has data check everyOne value", testValuesPopulated)
    suite.addTest("Testing HashMap is empty check values and set data check values again", testValuesEmptyThenPopulated)
    suite.addTest("Testing HashMap is empty and check if key is empty string", testReplaceEmptyMapEmptyKey)
    suite.addTest("Testing HashMap is empty and check if key is key", testReplaceEmptyMapNonExistKey)
    suite.addTest("Testing HashMap has data and check if key and value not change", testReplaceExistKeySameValue)
    suite.addTest("Testing HashMap has data and check if value change", testReplaceExistKeyDiffValue)
    suite.addTest("Testing HashMap has data and check if key is 1 and value is change", testReplaceNonExistNumKey)
    suite.addTest("Testing HashMap has object data and check if key is 1 and value is change",
        testReplaceExistNumKeyObjValue)
    suite.addTest("Testing HashMap has object data and check if key is existed and value is change",
        testReplaceExistObjKeyDiffObjValue)
    suite.addTest("Testing HashMap has object data and check if key not existed and value is change",
        testReplaceNonExistObjKey)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHMAP_CLEAR_0100
function testClearEmptyMap() {
    const hashMap = new HashMap<string, double>();
    hashMap.clear();
    arktest.assertEQ(hashMap.length, 0);
    arktest.assertEQ(hashMap.get('ji'), undefined);
    arktest.assertFalse(hashMap.hasKey('ji'), "Empty HashMap cleared, hasKey 'ji' is false");
}

//Test cases ported from ArkTS 1.0:HASHMAP_CLEAR_0200
function testClearPopulatedMap() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 12);
    hashMap.clear();
    arktest.assertEQ(hashMap.length, 0);
    arktest.assertEQ(hashMap.get('ji'), undefined);
    arktest.assertFalse(hashMap.hasKey('ji'), "Populated HashMap cleared, hasKey 'ji' is false");
}

//Test cases ported from ArkTS 1.0:HASHMAP_CLEAR_0300
function testClearSetSameKey() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 12);
    hashMap.clear();
    hashMap.set("ji", 13);
    arktest.assertEQ(hashMap.length, 1);
}

function testClearSetNewKey() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 12);
    hashMap.clear();
    hashMap.set("sparrow", 13);
    arktest.assertEQ(hashMap.length, 1);
}

//Test cases ported from ArkTS 1.0:HASHMAP_KEYS_0100
function testKeysEmpty() {
    const hashMap = new HashMap<string, double>();
    let keys = hashMap.keys();
    arktest.assertTrue(keys.next().done, "Empty HashMap: keys iterator .next().done is true");
}

//Test cases ported from ArkTS 1.0:HASHMAP_KEYS_0200
function testKeysPopulated() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 1);
    hashMap.set(1, 2);
    hashMap.set(2, 3);
    let keys = hashMap.keys();
    let temp = keys.next();
    let index = 0;
    while (!temp.done) {
        arktest.assertEQ(temp.value, index);
        temp = keys.next();
        index++;
    }
}

function testKeysEmptyThenPopulated() {
    const hashMap = new HashMap<double, double>();
    let result = hashMap.keys();
    arktest.assertTrue(result.next().done, "Empty HashMap: keys iterator .next().done is true");

    hashMap.set(3, 1);
    hashMap.set(4, 2);
    hashMap.set(5, 3);
    let keys = hashMap.keys();
    let temp = keys.next();
    let index = 3;
    while (!temp.done) {
        arktest.assertEQ(temp.value, index);
        temp = keys.next();
        index++;
    }
}

//Test cases ported from ArkTS 1.0:HASHMAP_VALUES_0100
function testValuesEmpty() {
    const hashMap = new HashMap<string, double>();
    let keys = hashMap.values();
    arktest.assertTrue(keys.next().done, "values().next().done should be true for an empty HashMap");
}

//Test cases ported from ArkTS 1.0:HASHMAP_VALUES_0200
function testValuesPopulated() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(0, 1);
    hashMap.set(1, 2);
    hashMap.set(2, 3);
    let values = hashMap.values();
    let temp = values.next();
    let index = 1;
    while (!temp.done) {
        arktest.assertEQ(temp.value, index);
        temp = values.next();
        index++;
    }
}

function testValuesEmptyThenPopulated() {
    const hashMap = new HashMap<string, double>();
    let result = hashMap.values();
    arktest.assertTrue(result.next().done, "values().next().done should be true for an empty HashMap");

    hashMap.set("ji", 1);
    hashMap.set("li", 2);
    hashMap.set("qi", 3);
    let values = hashMap.values();
    let temp = values.next();
    let index = 1;
    while (!temp.done) {
        arktest.assertEQ(temp.value, index);
        temp = values.next();
        index++;
    }
}

//Test cases ported from ArkTS 1.0:HASHMAP_REPLACE_0100
function testReplaceEmptyMapEmptyKey() {
    const hashMap = new HashMap<string, double>();
    arktest.assertFalse(hashMap.replace('', 1), "Replacing non-existent empty string key in empty map should fail");
}

//Test cases ported from ArkTS 1.0:HASHMAP_REPLACE_0400
function testReplaceEmptyMapNonExistKey() {
    const hashMap = new HashMap<string, double>();
    arktest.assertFalse(hashMap.replace('key', 1), "Replacing non-existent 'key' in empty map should fail");
}

//Test cases ported from ArkTS 1.0:HASHMAP_REPLACE_0500
function testReplaceExistKeySameValue() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1);
    arktest.assertTrue(hashMap.replace('ji', 1), "Replacing existing key with the same value should succeed");
}

//Test cases ported from ArkTS 1.0:HASHMAP_REPLACE_0600
function testReplaceExistKeyDiffValue() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1);
    arktest.assertTrue(hashMap.replace('ji', 2), "Replacing existing key with a different value should succeed");
}

//Test cases ported from ArkTS 1.0:HASHMAP_REPLACE_0700
function testReplaceNonExistNumKey() {
    const hashMap = new HashMap<double, double>();
    hashMap.set(2, 1);
    arktest.assertFalse(hashMap.replace(1, 2), "Replacing non-existent numeric key in populated map should fail");
}

class Tmp {
    name: number = 0
}
function testReplaceExistNumKeyObjValue() {
    const hashMap = new HashMap<double, Tmp>();
    let paramOne: Tmp = {
        name: 1
    };
    let paramTwo: Tmp = {
        name: 2
    };
    hashMap.set(1, paramOne);
    arktest.assertTrue(hashMap.replace(1, paramTwo), "Replacing existing key with a different value should succeed");
}

class Name {
    name: string = ""
}
function testReplaceExistObjKeyDiffObjValue() {
    const hashMap = new HashMap<Name, Name>();
    let paramOne: Name = {
        name: "Lily"
    };
    let paramTwo: Name = {
        name: "Judy"
    };
    let paramThree: Name = {
        name: "Mic"
    };
    hashMap.set(paramOne, paramTwo);
    arktest.assertTrue(hashMap.replace(paramOne, paramThree),
        "Replacing existing key with a different value should succeed");
}

class Age {
    age: number = 0
}
function testReplaceNonExistObjKey() {
    const hashMap = new HashMap<Age, double>();
    let paramOne: Age = {
        age: 18
    };
    let paramTwo: Age = {
        age: 20
    };
    hashMap.set(paramOne, 2);
    arktest.assertFalse(hashMap.replace(paramTwo, 4),
        "Replacing a non-existent object key in a populated map should return false");
}

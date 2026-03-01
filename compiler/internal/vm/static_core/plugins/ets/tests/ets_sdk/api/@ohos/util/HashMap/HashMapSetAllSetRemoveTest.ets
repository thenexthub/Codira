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
    const suite = new arktest.ArkTestsuite("HashMap SetAll Set Remove API tests")

    suite.addTest("Testing HashMap is empty check length", testLengthNewMap)
    suite.addTest("Testing HashMap has data check length", testSetAllWithNewDataCheckLength)
    suite.addTest("Testing HashMap has data check data is change", testSetAllPreservesOriginalValue)
    suite.addTest("Testing HashMap has data check same key diff value", testSetAllUpdatesExistingKey)
    suite.addTest("Testing HashMap remove->set->clear->replace", testGetUndefinedAfterRemoveSetClearReplace)
    suite.addTest("Testing HashMap has data and setAll same data check length", testSetAllWithSameDataCheckLength)
    suite.addTest("Testing HashMap set empty string check length and get empty string", testSetEmptyStringKeyCheckState)
    suite.addTest("Testing HashMap set a check length and get a", testSetSingleCharKeyCheckState)
    suite.addTest("Testing HashMap set key length 1024 check hashMap length and get key", testSetLongKeyCheckState)
    suite.addTest("Testing HashMap set value is 1 check hashMap length and get value", testSetSimpleKeyValueCheckState)
    suite.addTest("Testing HashMap data has 1000 check length =1 and has 1000 data", testSetMultipleItemsCheckState)
    suite.addTest("Testing HashMap remove->set->clear->replace", testGetUndefinedAfterRemoveSetClearReplaceAgain)
    suite.addTest("Testing HashMap is empty and check if remove empty string", testRemoveFromEmptyMapEmptyString)
    suite.addTest("Testing HashMap is empty and check if remove key", testRemoveFromEmptyMapNonExistentKey)
    suite.addTest("Testing HashMap has data and check if remove this key", testRemoveExistingKeyFromPopulatedMap)
    suite.addTest("Testing HashMap has data and check if not remove this key", testRemoveNonExistentKeyFromPopulatedMap)
    suite.addTest("Testing HashMap remove->set->clear", testRemoveAfterOpsAndClear)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_ALL_0100
function testLengthNewMap() {
    const hashMap = new HashMap<string, string>();
    arktest.assertEQ(hashMap.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_ALL_0200
function testSetAllWithNewDataCheckLength() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    hashMap.set("sparrow", 356);
    const newHashMap = new HashMap<string, double>();
    newHashMap.set("newMap", 99);
    hashMap.setAll(newHashMap);
    arktest.assertEQ(hashMap.length, 3);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_ALL_0300
function testSetAllPreservesOriginalValue() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    hashMap.set("sparrow", 356);
    const newHashMap = new HashMap<string, double>();
    newHashMap.set("newMap", 100);
    hashMap.setAll(newHashMap);
    arktest.assertEQ(hashMap.get("sparrow"), 356);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_ALL_0400
function testSetAllUpdatesExistingKey() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    hashMap.set("sparrow", 356);
    const newHashMap = new HashMap<string, double>();
    newHashMap.set("sparrow", 100);
    hashMap.setAll(newHashMap);
    arktest.assertEQ(hashMap.get("sparrow"), 100);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_ALL_0500
function testGetUndefinedAfterRemoveSetClearReplace() {
    const hashMap = new HashMap<string, double>();
    hashMap.remove("sparrow");
    hashMap.set("sparrow", 356);
    hashMap.clear();
    hashMap.replace("sparrow", 12);
    arktest.assertEQ(hashMap.get("sparrow"), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_ALL_0600
function testSetAllWithSameDataCheckLength() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("sparrow", 356);
    const newHashMap = new HashMap<string, double>();
    newHashMap.set("sparrow", 356);
    hashMap.setAll(newHashMap);
    arktest.assertEQ(hashMap.length, 1);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_0100
function testSetEmptyStringKeyCheckState() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("", 356);
    arktest.assertEQ(hashMap.length, 1);
    arktest.assertEQ(hashMap.get(""), 356);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_0200
function testSetSingleCharKeyCheckState() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("a", 356);
    arktest.assertEQ(hashMap.length, 1);
    arktest.assertEQ(hashMap.get("a"), 356);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_0500
function testSetLongKeyCheckState() {
    const hashMap = new HashMap<string, double>();
    let param = 'a'.repeat(2048);
    hashMap.set(param, 356);
    arktest.assertEQ(hashMap.length, 1);
    arktest.assertEQ(hashMap.get(param), 356);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_0700
function testSetSimpleKeyValueCheckState() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1);
    arktest.assertEQ(hashMap.length, 1);
    arktest.assertEQ(hashMap.get("ji"), 1);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_0900
function testSetMultipleItemsCheckState() {
    const hashMap = new HashMap<string, double>();
    for (let i: int = 0; i < 1000; i++) {
        hashMap.set("ji" + i, i);
    }
    for (let i: int = 0; i < 1000; i++) {
        arktest.assertEQ(hashMap.get("ji" + i), i);
    }
    arktest.assertEQ(hashMap.length, 1000);
}

//Test cases ported from ArkTS 1.0:HASHMAP_SET_1000
function testGetUndefinedAfterRemoveSetClearReplaceAgain() {
    const hashMap = new HashMap<string, double>();
    hashMap.remove("sparrow");
    hashMap.set("sparrow", 356);
    hashMap.clear();
    hashMap.replace("sparrow", 12);
    arktest.assertEQ(hashMap.get("sparrow"), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_REMOVE_0100
function testRemoveFromEmptyMapEmptyString() {
    const hashMap = new HashMap<string, string>();
    arktest.assertEQ(hashMap.remove(''), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_REMOVE_0400
function testRemoveFromEmptyMapNonExistentKey() {
    const hashMap = new HashMap<string, string>();
    arktest.assertEQ(hashMap.remove('key'), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_REMOVE_0500
function testRemoveExistingKeyFromPopulatedMap() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertEQ(hashMap.remove('ji'), "hello");
}

//Test cases ported from ArkTS 1.0:HASHMAP_REMOVE_0600
function testRemoveNonExistentKeyFromPopulatedMap() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertEQ(hashMap.remove('li'), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_REMOVE_0700
function testRemoveAfterOpsAndClear() {
    const hashMap = new HashMap<string, string>();
    hashMap.remove("ji");
    hashMap.set("ji", "hello");
    hashMap.clear();
    arktest.assertEQ(hashMap.remove('ji'), undefined);
}

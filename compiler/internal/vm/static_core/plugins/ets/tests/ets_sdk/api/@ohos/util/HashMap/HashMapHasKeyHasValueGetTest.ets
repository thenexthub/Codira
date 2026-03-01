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
    const suite = new arktest.ArkTestsuite("HashMap HasKey HasValue Get API tests")

    suite.addTest("Testing HashMap is empty and check if empty string has", testHasKeyEmptyMapEmptyString)
    suite.addTest("Testing HashMap is empty and check if key has", testHasKeyEmptyMapNonExistentKey)
    suite.addTest("Testing HashMap has data and check if key has", testHasKeyPopulatedMapExistingKey)
    suite.addTest("Testing HashMap has data and check if not this key has", testHasKeyPopulatedMapNonExistentKey)
    suite.addTest("Testing HashMap has data and check if value has", testHasKeyPopulatedMapValueAsKey)
    suite.addTest("Testing HashMap remove->set->clear", testHasKeyAfterRemoveSetClear)
    suite.addTest("Testing HashMap is empty and check if empty string has", testHasValueEmptyMap)
    suite.addTest("Testing HashMap is empty and check if key has", testHasValueEmptyMapNonExistentValue)
    suite.addTest("Testing HashMap has data and check if value has", testHasValuePopulatedMapExistingValue)
    suite.addTest("Testing HashMap has data and check if not this value has", testHasValuePopulatedMapNonExistentValue)
    suite.addTest("Testing HashMap remove->set->clear", testHasValueAfterRemoveSetClear)
    suite.addTest("Testing HashMap is empty and check if get empty string", testGetEmptyMapEmptyStringKey)
    suite.addTest("Testing HashMap is empty and check if get key", testGetEmptyMapNonExistentKey)
    suite.addTest("Testing HashMap has data and check if get this key", testGetPopulatedMapExistingKey)
    suite.addTest("Testing HashMap has data and check if get not this key", testGetPopulatedMapNonExistentKey)
    suite.addTest("Testing HashMap remove->set->clear", testGetAfterRemoveSetClear)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_KEY_0100
function testHasKeyEmptyMapEmptyString() {
    const hashMap = new HashMap<string, double>();
    arktest.assertFalse(hashMap.hasKey(""), "hasKey('') on an empty HashMap should return false");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_KEY_0400
function testHasKeyEmptyMapNonExistentKey() {
    const hashMap = new HashMap<string, double>();
    arktest.assertFalse(hashMap.hasKey('key'), "hasKey('key') on an empty HashMap should return false");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_KEY_0500
function testHasKeyPopulatedMapExistingKey() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1);
    arktest.assertTrue(hashMap.hasKey('ji'), "hasKey('ji') should return true when the key exists");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_KEY_0600
function testHasKeyPopulatedMapNonExistentKey() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1);
    arktest.assertFalse(hashMap.hasKey('li'), "hasKey('li') should return false when the key does not exist");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_KEY_0700
function testHasKeyPopulatedMapValueAsKey() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertFalse(hashMap.hasKey('hello'),
        "hasKey('hello') should return false when 'hello' is a value, not a key");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_KEY_0800
function testHasKeyAfterRemoveSetClear() {
    const hashMap = new HashMap<string, string>();
    hashMap.remove("ji");
    hashMap.set("ji", "hello");
    hashMap.clear();
    arktest.assertFalse(hashMap.hasKey('hello'), "hasKey('hello') should return false after HashMap is cleared");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_VALUE_0100
function testHasValueEmptyMap() {
    const hashMap = new HashMap<string, string>();
    arktest.assertFalse(hashMap.hasValue('hello'), "hasValue('hello') on an empty HashMap should return false");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_VALUE_0400
function testHasValueEmptyMapNonExistentValue() {
    const hashMap = new HashMap<string, string>();
    arktest.assertFalse(hashMap.hasValue('key'), "hasValue('key') on an empty HashMap should return false");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_VALUE_0500
function testHasValuePopulatedMapExistingValue() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertTrue(hashMap.hasValue('hello'), "hasValue('hello') should return true when the value exists");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_VALUE_0600
function testHasValuePopulatedMapNonExistentValue() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertFalse(hashMap.hasValue('li'), "hasValue('li') should return false when the value does not exist");
}

//Test cases ported from ArkTS 1.0:HASHMAP_HAS_VALUE_0700
function testHasValueAfterRemoveSetClear() {
    const hashMap = new HashMap<string, string>();
    hashMap.remove("ji");
    hashMap.set("ji", "hello");
    hashMap.clear();
    arktest.assertFalse(hashMap.hasValue('hello'), "hasValue('hello') should return false after HashMap is cleared");
}

//Test cases ported from ArkTS 1.0:HASHMAP_GET_0100
function testGetEmptyMapEmptyStringKey() {
    const hashMap = new HashMap<string, string>();
    arktest.assertEQ(hashMap.get(''), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_GET_0400
function testGetEmptyMapNonExistentKey() {
    const hashMap = new HashMap<string, string>();
    arktest.assertEQ(hashMap.get('key'), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_GET_0500
function testGetPopulatedMapExistingKey() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertEQ(hashMap.get('ji'), "hello");
}

//Test cases ported from ArkTS 1.0:HASHMAP_GET_0600
function testGetPopulatedMapNonExistentKey() {
    const hashMap = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    arktest.assertEQ(hashMap.get('li'), undefined);
}

//Test cases ported from ArkTS 1.0:HASHMAP_GET_0700
function testGetAfterRemoveSetClear() {
    const hashMap = new HashMap<string, string>();
    hashMap.remove("ji");
    hashMap.set("ji", "hello");
    hashMap.clear();
    arktest.assertEQ(hashMap.get('ji'), undefined);
}

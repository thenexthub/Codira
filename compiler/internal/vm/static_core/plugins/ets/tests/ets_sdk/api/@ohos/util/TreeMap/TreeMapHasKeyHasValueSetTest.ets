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

import TreeMap from "@ohos.util.TreeMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeMap HasKey HasValue Set API tests")

    suite.addTest("Test TreeMap hasKey() with an empty string key on an empty map.",
        testHasKeyWithEmptyStringKeyOnEmptyMap)
    suite.addTest("Test TreeMap hasKey() with a non-existing key on an empty map.",
        testHasKeyWithNonExistingKeyOnEmptyMap)
    suite.addTest("Test TreeMap hasKey() with an existing key.", testHasKeyWithExistingKey)
    suite.addTest("Test TreeMap hasKey() with a non-existing key on a non-empty map.",
        testHasKeyWithNonExistingKeyOnNonEmptyMap)
    suite.addTest("Test TreeMap hasKey() with a key that is a string representation of a double.",
        testHasKeyWithKeyAsStringRepresentationOfNumber)
    suite.addTest("Test TreeMap hasValue() with an existing value.", testHasValueWithExistingValue)
    suite.addTest("Test TreeMap hasValue() with a non-existing value.", testHasValueWithNonExistingValue)
    suite.addTest("Test TreeMap hasValue() with an existing value when multiple entries are present.",
        testHasValueWithExistingValueMultipleEntries)
    suite.addTest("Test TreeMap set() with an empty string key, then check length and get().",
        testSetWithEmptyStringKey)
    suite.addTest("Test TreeMap set() with a simple string key, then check length and get().",
        testSetWithSimpleStringKey)
    suite.addTest("Test TreeMap set() with a simple string key and different value, then check length and get().",
        testSetWithSimpleStringKeyDifferentValue)
    suite.addTest("Test TreeMap set() by adding multiple distinct keys in a loop check length and get() for all keys.",
        testSetMultipleDistinctKeysInLoop)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREEMAP_0500
function testHasKeyWithEmptyStringKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.hasKey("");
    arktest.assertFalse(result, "Expected hasKey('') on an empty map to return false");
}

//Test cases ported from ArkTS 1.0:TREEMAP_0800
function testHasKeyWithNonExistingKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.hasKey("squirrel");
    arktest.assertFalse(result, "Expected hasKey('squirrel') for a non-existing key on an empty map to return false");
}

//Test cases ported from ArkTS 1.0:TREEMAP_0900
function testHasKeyWithExistingKey() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.hasKey("squirrel");
    arktest.assertTrue(result, "Expected hasKey('squirrel') to return true for an existing key");
}

//Test cases ported from ArkTS 1.0:TREEMAP_1000
function testHasKeyWithNonExistingKeyOnNonEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.hasKey('goose');
    arktest.assertFalse(result, "Expected hasKey('goose') to return false for a non-existing key on a non-empty map");
}

//Test cases ported from ArkTS 1.0:TREEMAP_1100
function testHasKeyWithKeyAsStringRepresentationOfNumber() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.hasKey('123');
    arktest.assertFalse(result, "Expected hasKey('123') to return false when '123' is not a key");
}

//Test cases ported from ArkTS 1.0:TREEMAP_1500
function testHasValueWithExistingValue() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.hasValue(123);
    arktest.assertTrue(result, "Expected hasValue(123) to return true for an existing value");
}

//Test cases ported from ArkTS 1.0:TREEMAP_1600
function testHasValueWithNonExistingValue() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.hasValue(333);
    arktest.assertFalse(result, "Expected hasValue(333) to return false for a non-existing value");
}

//Test cases ported from ArkTS 1.0:TREEMAP_1700
function testHasValueWithExistingValueMultipleEntries() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    treeMap.set("sparrow", 356);
    let result = treeMap.hasValue(356);
    arktest.assertTrue(result, "Expected hasValue(356) to return true when the value exists among multiple entries");
}

//Test cases ported from ArkTS 1.0:REEMAP_4000
function testSetWithEmptyStringKey() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("", 123);
    arktest.assertEQ(treeMap.length, 1);
    let result = treeMap.get("");
    arktest.assertEQ(result, 123);
}

//Test cases ported from ArkTS 1.0:REEMAP_4100
function testSetWithSimpleStringKey() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("a", 123);
    arktest.assertEQ(treeMap.length, 1);
    let result = treeMap.get("a");
    arktest.assertEQ(result, 123);
}

//Test cases ported from ArkTS 1.0:REEMAP_4500
function testSetWithSimpleStringKeyDifferentValue() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("a", 1);
    arktest.assertEQ(treeMap.length, 1);
    let result = treeMap.get("a");
    arktest.assertEQ(result, 1);
}

//Test cases ported from ArkTS 1.0:REEMAP_4700
function testSetMultipleDistinctKeysInLoop() {
    let arr = 'abc';
    let treeMap = new TreeMap<string, double>();
    for (let i: int = 0; i < 1000; i++) {
        treeMap.set(arr + i, 123);
    }
    arktest.assertEQ(treeMap.length, 1000);
    for (let k = 0; k < 1000; k++) {
        let result = treeMap.get(arr + k);
        arktest.assertEQ(result, 123);
    }
}

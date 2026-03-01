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
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeMap SetAll GetLowerKey GetHigherKey API tests")

    suite.addTest("Test the function of the interface SetAll.", testInitialEmptyMapLength)
    suite.addTest("Test TreeMap setAll() by adding one element and checking length.", testSetSingleElement)
    suite.addTest("Test TreeMap setAll() by adding multiple elements and checking length.", testSetMultipleElements)
    suite.addTest("Test TreeMap setAll() with a custom comparator, check length, first key, and last key.",
        testSetWithCustomComparator)
    suite.addTest("Test TreeMap setAll() by copying one map to another (both with custom comparators) check length.",
        testSetAllFromMapWithCustomComparators)
    suite.addTest("Test TreeMap setAll() by copying a map into another map that already contains distinct elements.",
        testSetAllToNonEmptyMap)
    suite.addTest("Test TreeMap getLowerKey() with an empty string key on an empty map.",
        testGetLowerKeyEmptyStringOnEmptyMap)
    suite.addTest("Test TreeMap getLowerKey() with a non-existing key on an empty map.",
        testGetLowerKeyNonExistingKeyOnEmptyMap)
    suite.addTest("Test TreeMap getLowerKey() with the only key in the map.", testGetLowerKeyWithOnlyKeyInMap)
    suite.addTest("Test TreeMap getLowerKey() with an existing key that has a lower key.",
        testGetLowerKeyExistingKeyWithLower)
    suite.addTest("Test TreeMap getLowerKey() with a key that is not in the map but has a lower key.",
        testGetLowerKeyNonExistingKeyWithLower)
    suite.addTest("Test TreeMap getHigherKey() with an empty string key on an empty map.",
        testGetHigherKeyEmptyStringOnEmptyMap)
    suite.addTest("Test TreeMap getHigherKey() with a non-existing key on an empty map.",
        testGetHigherKeyNonExistingKeyOnEmptyMap)
    suite.addTest("Test TreeMap getHigherKey() with the only key in the map.", testGetHigherKeyWithOnlyKeyInMap)
    suite.addTest("Test TreeMap getHigherKey() with an existing key that has a higher key.",
        testGetHigherKeyExistingKeyWithHigher)
    suite.addTest("Test TreeMap getHigherKey() with a key that is not in the map but has a higher key.",
        testGetHigherKeyNonExistingKeyWithHigher)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREEMAP_3400
function testInitialEmptyMapLength() {
    let treeMap = new TreeMap<string, double>();
    arktest.assertEQ(treeMap.length, 0);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3500
function testSetSingleElement() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("HIG", 123);
    arktest.assertEQ(treeMap.length, 1);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3600
function testSetMultipleElements() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("ABC", 123);
    treeMap.set("DEF", 456);
    treeMap.set("GHI", 789);
    treeMap.set("JKL", 222);
    treeMap.set("MNP", 333);
    arktest.assertEQ(treeMap.length, 5);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3700
function testSetWithCustomComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("HIG", "456");
    treeMap.set("DEF", "123");
    treeMap.set("XYZ", "789");
    arktest.assertEQ(treeMap.length, 3);
    let value1 = treeMap.getFirstKey();
    let value2 = treeMap.getLastKey();
    arktest.assertEQ(value1, 'DEF');
    arktest.assertEQ(treeMap.get(value1!), "123");
    arktest.assertEQ(value2, 'XYZ');
    arktest.assertEQ(treeMap.get(value2!), "789");
}

//Test cases ported from ArkTS 1.0:TREEMAP_3800
function testSetAllFromMapWithCustomComparators() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("HIG", "456");
    treeMap.set("DEF", "123");
    treeMap.set("XYZ", "789");
    let newMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    newMap.clear();
    newMap.setAll(treeMap);
    arktest.assertEQ(newMap.length, 3);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3900
function testSetAllToNonEmptyMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("HIG", "456");
    treeMap.set("DEF", "123");
    treeMap.set("XYZ", "789");
    let newMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    newMap.clear();
    newMap.set("demo", '123');
    newMap.setAll(treeMap);
    arktest.assertEQ(newMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_5400
function testGetLowerKeyEmptyStringOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    try {
        let result = treeMap.getLowerKey("");
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_5700
function testGetLowerKeyNonExistingKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    try {
        let result = treeMap.getLowerKey("key");
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_5800
function testGetLowerKeyWithOnlyKeyInMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    try {
        let result = treeMap.getLowerKey("squirrel");
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_5900
function testGetLowerKeyExistingKeyWithLower() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("BBB", 123);
    treeMap.set("AAA", 456);
    treeMap.set("CCC", 789);
    let result = treeMap.getLowerKey("CCC");
    arktest.assertEQ(result, "BBB");
    arktest.assertEQ(treeMap.get(result!), 123);
}

//Test cases ported from ArkTS 1.0:TREEMAP_6000
function testGetLowerKeyNonExistingKeyWithLower() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("BBB", 123);
    treeMap.set("AAA", 456);
    treeMap.set("CCC", 789);
    let result = treeMap.getLowerKey("AAA" + "1");
    arktest.assertEQ(result, "AAA");
    arktest.assertEQ(treeMap.get(result!), 456);
}

//Test cases ported from ArkTS 1.0:TREEMAP_6100
function testGetHigherKeyEmptyStringOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    try {
        let result = treeMap.getHigherKey("");
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_6400
function testGetHigherKeyNonExistingKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    try {
        let result = treeMap.getHigherKey('key');
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_6500
function testGetHigherKeyWithOnlyKeyInMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("AAA", 123);
    try {
        let result = treeMap.getHigherKey('AAA');
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_6600
function testGetHigherKeyExistingKeyWithHigher() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("BBB", 123);
    treeMap.set("AAA", 456);
    treeMap.set("CCC", 789);
    let result = treeMap.getHigherKey("BBB");
    arktest.assertEQ(result, "CCC");
    arktest.assertEQ(treeMap.get(result!), 789);
}

//Test cases ported from ArkTS 1.0:TREEMAP_6700
function testGetHigherKeyNonExistingKeyWithHigher() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("BBB", 123);
    treeMap.set("AAA", 456);
    treeMap.set("CCC", 789);
    let result = treeMap.getHigherKey("BBB" + "1");
    arktest.assertEQ(result, "CCC");
    arktest.assertEQ(treeMap.get(result!), 789);
}

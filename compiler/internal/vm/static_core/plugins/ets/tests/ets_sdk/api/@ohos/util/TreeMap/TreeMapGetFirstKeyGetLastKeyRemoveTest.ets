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
import {BusinessError} from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeMap GetFirstKey GetLastKey Remove API tests")

    suite.addTest("Test TreeMap getFirstKey() on an empty map.", testGetFirstKeyOnEmptyMap)
    suite.addTest("Test TreeMap getFirstKey() on a map with a single element.", testGetFirstKeyOnMapWithSingleElement)
    suite.addTest("Test TreeMap getFirstKey() on a map with multiple elements (natural string order).",
        testGetFirstKeyOnMapWithMultipleElementsNaturalOrder)
    suite.addTest("Test TreeMap getFirstKey() with a custom reverse string comparator.",
        testGetFirstKeyWithCustomReverseComparator)
    suite.addTest("Test TreeMap getFirstKey() with a custom natural string order comparator.",
        testGetFirstKeyWithCustomNaturalOrderComparator)
    suite.addTest("Test TreeMap getLastKey() on an empty map.", testGetLastKeyOnEmptyMap)
    suite.addTest("Test TreeMap getLastKey() on a map with a single element.", testGetLastKeyOnMapWithSingleElement)
    suite.addTest("Test TreeMap getLastKey() on a map with multiple elements (natural string order).",
        testGetLastKeyOnMapWithMultipleElementsNaturalOrder)
    suite.addTest("Test TreeMap getLastKey() with a custom natural string order comparator and check length.",
        testGetLastKeyWithCustomNaturalOrderComparator)
    suite.addTest("Test TreeMap getLastKey() with custom comparator, clear, re-populate, and check getLastKey() again.",
        testGetLastKeyWithCustomComparatorClearRepopulate)
    suite.addTest("Test TreeMap remove() with a non-existing empty string key on a non-empty map.",
        testRemoveNonExistingEmptyStringKeyOnNonEmptyMap)
    suite.addTest("Test TreeMap remove() with a non-existing key on an empty map.", testRemoveNonExistingKeyOnEmptyMap)
    suite.addTest("Test TreeMap remove() with an existing key and check returned value.", testRemoveExistingKey)
    suite.addTest("Test TreeMap remove() with a non-existing key on a non-empty map.",
        testRemoveNonExistingKeyOnNonEmptyMap)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREEMAP_2400
function testGetFirstKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    try {
        let result = treeMap.getFirstKey();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_2500
function testGetFirstKeyOnMapWithSingleElement() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("sparrow", 356);
    let result = treeMap.getFirstKey();
    arktest.assertEQ(result, 'sparrow');
    arktest.assertEQ(treeMap.get(result!), 356);
}

//Test cases ported from ArkTS 1.0:TREEMAP_2600
function testGetFirstKeyOnMapWithMultipleElementsNaturalOrder() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("DEF", 123);
    treeMap.set("ABC", 356);
    let result = treeMap.getFirstKey();
    arktest.assertEQ(result, 'ABC');
    arktest.assertEQ(treeMap.get(result!), 356);
}

//Test cases ported from ArkTS 1.0:TREEMAP_2700
function testGetFirstKeyWithCustomReverseComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set("aa", "3");
    treeMap.set("dd", "1");
    treeMap.set("cc", "2");
    treeMap.set("bb", "4");
    let result = treeMap.getFirstKey();
    arktest.assertEQ(result, 'dd');
    arktest.assertEQ(treeMap.get(result!), "1");
}

//Test cases ported from ArkTS 1.0:TREEMAP_2800
function testGetFirstKeyWithCustomNaturalOrderComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    treeMap.set("aa", "3");
    treeMap.set("dd", "1");
    treeMap.set("cc", "2");
    treeMap.set("bb", "4");
    let result = treeMap.getFirstKey();
    arktest.assertEQ(result, 'aa');
    arktest.assertEQ(treeMap.get(result!), "3");
}

//Test cases ported from ArkTS 1.0:TREEMAP_2900
function testGetLastKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    try {
        let result = treeMap.getLastKey();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREEMAP_3000
function testGetLastKeyOnMapWithSingleElement() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("DEF", 123);
    let result = treeMap.getLastKey();
    arktest.assertEQ(result, 'DEF');
    arktest.assertEQ(treeMap.get(result!), 123);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3100
function testGetLastKeyOnMapWithMultipleElementsNaturalOrder() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("HIG", 456);
    treeMap.set("DEF", 123);
    let result = treeMap.getLastKey();
    arktest.assertEQ(result, 'HIG');
    arktest.assertEQ(treeMap.get(result!), 456);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3200
function testGetLastKeyWithCustomNaturalOrderComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    treeMap.set("HIG", "456");
    treeMap.set("DEF", "123");
    treeMap.set("XYZ", "789");
    let result = treeMap.getLastKey();
    arktest.assertEQ(result, 'XYZ');
    arktest.assertEQ(treeMap.get(result!), "789");
    arktest.assertEQ(treeMap.length, 3);
}

//Test cases ported from ArkTS 1.0:TREEMAP_3300
function testGetLastKeyWithCustomComparatorClearRepopulate() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    treeMap.set("HIG", "456");
    treeMap.set("DEF", "123");
    treeMap.set("XYZ", "789");
    let result = treeMap.getLastKey();
    arktest.assertEQ(result, 'XYZ');
    arktest.assertEQ(treeMap.get(result!), "789");
    arktest.assertEQ(treeMap.length, 3);
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
    treeMap.set("ABC", '111');
    treeMap.set("JKL", '222');
    result = treeMap.getLastKey();
    arktest.assertEQ(result, 'JKL');
    arktest.assertEQ(treeMap.get(result!), '222');
}

//Test cases ported from ArkTS 1.0:TREEMAP_4800
function testRemoveNonExistingEmptyStringKeyOnNonEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    treeMap.set("sparrow", 456);
    let result = treeMap.remove("");
    arktest.assertEQ(result, undefined);
}

//Test cases ported from ArkTS 1.0:TREEMAP_5100
function testRemoveNonExistingKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.remove('key');
    arktest.assertEQ(result, undefined);
}

//Test cases ported from ArkTS 1.0:TREEMAP_5200
function testRemoveExistingKey() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    treeMap.set("sparrow", 456);
    let result = treeMap.remove("sparrow");
    arktest.assertEQ(result, 456);
}

//Test cases ported from ArkTS 1.0:TREEMAP_5300
function testRemoveNonExistingKeyOnNonEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    treeMap.set("sparrow", 456);
    let result = treeMap.remove("tiger");
    arktest.assertEQ(result, undefined);
}

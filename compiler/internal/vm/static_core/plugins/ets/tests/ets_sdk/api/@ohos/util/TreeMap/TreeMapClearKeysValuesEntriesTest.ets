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
    const suite = new arktest.ArkTestsuite("TreeMap Clear Keys Values Entries API tests")

    suite.addTest("Test the function of the interface clear.", testInitialEmptyMapProperties)
    suite.addTest("Test TreeMap clear() on a non-empty map and check properties.", testClearOnNonEmptyMap)
    suite.addTest("Test TreeMap clear() on a map with a custom comparator and check properties.",
        testClearWithCustomComparator)
    suite.addTest("Test TreeMap clear(), re-populate, and check properties with a custom comparator.",
        testClearAndRepopulateWithCustomComparator)
    suite.addTest("Test TreeMap keys() on an empty map.", testKeysOnEmptyMap)
    suite.addTest("Test TreeMap keys() on a map with multiple elements (natural string order).",
        testKeysWithMultipleElementsNaturalOrder)
    suite.addTest("Test TreeMap keys() on a map with a custom reverse string comparator.",
        testKeysWithCustomReverseComparator)
    suite.addTest("Test TreeMap values() on an empty map.", testValuesOnEmptyMap)
    suite.addTest("Test TreeMap values() on a map with multiple elements (natural string order).",
        testValuesWithMultipleElementsNaturalOrder)
    suite.addTest("Test TreeMap values() on a map with a custom reverse string comparator.",
        testValuesWithCustomReverseComparator)
    suite.addTest("Test TreeMap entries() on an empty map.", testEntriesOnEmptyMap)
    suite.addTest("Test TreeMap entries(), then check values and length (natural string order).",
        testEntriesNaturalOrder)
    suite.addTest("Test TreeMap entries() with a custom reverse string comparator, then check values and length.",
        testEntriesCustomReverseComparator)
    suite.addTest("Test TreeMap entries(), remove an element, then check values and length.", testEntriesAfterRemove)
    suite.addTest("Test TreeMap entries(), replace an element's value, then check values and length.",
        testEntriesAfterReplace)
    suite.addTest("Test TreeMap entries(), clear the map, then check length.", testEntriesAfterClear)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREEMAP_7500
function testInitialEmptyMapProperties() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.length;
    arktest.assertEQ(result, 0);
    let result2 = treeMap.get('');
    arktest.assertEQ(result2, undefined);
    let result1 = treeMap.hasKey('');
    arktest.assertFalse(result1, "Expected hasKey('') on an empty map to return false");
}

//Test cases ported from ArkTS 1.0:TREEMAP_7600
function testClearOnNonEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    treeMap.set("sparrow", 356);
    treeMap.clear();
    let result = treeMap.length;
    arktest.assertEQ(result, 0);
    let result2 = treeMap.get('squirrel');
    arktest.assertEQ(result2, undefined);
    result2 = treeMap.get('sparrow');
    arktest.assertEQ(result2, undefined);
    let result1 = treeMap.hasKey('squirrel');
    arktest.assertFalse(result1, "Expected hasKey('squirrel') to return false after clear()");
    result1 = treeMap.hasKey('sparrow');
    arktest.assertFalse(result1, "Expected hasKey('sparrow') to return false after clear()");
}

//Test cases ported from ArkTS 1.0:TREEMAP_7700
function testClearWithCustomComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set("aa", "3");
    treeMap.set("dd", "1");
    treeMap.set("cc", "2");
    treeMap.set("bb", "4");
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
    let result = treeMap.get('aa');
    arktest.assertEQ(result, undefined);
    result = treeMap.get('cc');
    arktest.assertEQ(result, undefined);
    let result1 = treeMap.hasKey('bb');
    arktest.assertFalse(result1, "Expected hasKey('bb') to return false after clear() with custom comparator");
    result1 = treeMap.hasKey('dd');
    arktest.assertFalse(result1, "Expected hasKey('dd') to return false after clear() with custom comparator");
}

//Test cases ported from ArkTS 1.0:TREEMAP_7800
function testClearAndRepopulateWithCustomComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set("aa", "3");
    treeMap.set("dd", "1");
    treeMap.set("cc", "2");
    treeMap.set("bb", "4");
    treeMap.clear();
    treeMap.set("AAA", "111");
    treeMap.set("BBB", "222");
    treeMap.set("CCC", "333");
    arktest.assertEQ(treeMap.length, 3);
    let result = treeMap.getFirstKey();
    arktest.assertEQ(result, "CCC");
    arktest.assertEQ(treeMap.get(result!), "333");
    result = treeMap.getLastKey();
    arktest.assertEQ(result, "AAA");
    arktest.assertEQ(treeMap.get(result!), "111");
}

//Test cases ported from ArkTS 1.0:TREEMAP_7900
function testKeysOnEmptyMap() {
    let treeMap = new TreeMap<string, string>();
    let res = treeMap.keys();
    arktest.assertTrue(res.next().done, "Expected keys() iterator on an empty map to be done immediately");
}

//Test cases ported from ArkTS 1.0:TREEMAP_8000
function testKeysWithMultipleElementsNaturalOrder() {
    let treeMap = new TreeMap<string, string>();
    treeMap.set('AAA', "123");
    treeMap.set('BBB', "456");
    treeMap.set('CCC', "789");
    treeMap.set('DDD', "666");
    let res = treeMap.keys();
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertTrue(res.next().done, "Expected keys() iterator to be done after iterating all elements");
}

//Test cases ported from ArkTS 1.0:TREEMAP_8100
function testKeysWithCustomReverseComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set('AAA', "123");
    treeMap.set('BBB', "456");
    treeMap.set('CCC', "789");
    treeMap.set('DDD', "666");
    let res = treeMap.keys();
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertTrue(res.next().done, "Expected keys() iterator to be done after iterating all elements");
}

//Test cases ported from ArkTS 1.0:TREEMAP_8200
function testValuesOnEmptyMap() {
    let treeMap = new TreeMap<string, string>();
    let res = treeMap.values();
    arktest.assertTrue(res.next().done, "Expected values() iterator on an empty map to be done immediately");
}

//Test cases ported from ArkTS 1.0:TREEMAP_8300
function testValuesWithMultipleElementsNaturalOrder() {
    let treeMap = new TreeMap<string, string>();
    treeMap.set('AAA', "123");
    treeMap.set('BBB', "456");
    treeMap.set('CCC', "789");
    treeMap.set('DDD', "666");
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, '123');
    arktest.assertEQ(res.next().value, '456');
    arktest.assertEQ(res.next().value, '789');
    arktest.assertEQ(res.next().value, '666');
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
}

//Test cases ported from ArkTS 1.0:TREEMAP_8400
function testValuesWithCustomReverseComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set('AAA', "123");
    treeMap.set('BBB', "456");
    treeMap.set('CCC', "789");
    treeMap.set('DDD', "666");
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, '666');
    arktest.assertEQ(res.next().value, '789');
    arktest.assertEQ(res.next().value, '456');
    arktest.assertEQ(res.next().value, '123');
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
}

//Test cases ported from ArkTS 1.0:TREEMAP_9100
function testEntriesOnEmptyMap() {
    let treeMap = new TreeMap<string, string>();
    let res = treeMap.entries();
    arktest.assertTrue(res.next().done, "Expected entries() iterator on an empty map to be done immediately");
}

//Test cases ported from ArkTS 1.0:TREEMAP_9200
function testEntriesNaturalOrder() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    treeMap.entries();
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, 123);
    arktest.assertEQ(res.next().value, 456);
    arktest.assertEQ(res.next().value, 789);
    arktest.assertEQ(res.next().value, 666);
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_9300
function testEntriesCustomReverseComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set('AAA', '123');
    treeMap.set('BBB', '456');
    treeMap.set('CCC', '789');
    treeMap.set('DDD', '666');
    treeMap.entries();
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, '666');
    arktest.assertEQ(res.next().value, '789');
    arktest.assertEQ(res.next().value, '456');
    arktest.assertEQ(res.next().value, '123');
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_9400
function testEntriesAfterRemove() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    treeMap.entries();
    treeMap.remove("BBB");
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, 123);
    arktest.assertEQ(res.next().value, 789);
    arktest.assertEQ(res.next().value, 666);
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 3);
}

//Test cases ported from ArkTS 1.0:TREEMAP_9500
function testEntriesAfterReplace() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    treeMap.entries();
    treeMap.replace("BBB", 555);
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, 123);
    arktest.assertEQ(res.next().value, 555);
    arktest.assertEQ(res.next().value, 789);
    arktest.assertEQ(res.next().value, 666);
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_9600
function testEntriesAfterClear() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    treeMap.entries();
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
}
